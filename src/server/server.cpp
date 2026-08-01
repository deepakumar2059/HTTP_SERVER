#include"server.h"
#include"../network/socket.h"
#include<sys/socket.h>
#include<iostream>
#include<unistd.h>
#include<sys/epoll.h>
#include<fcntl.h>
#include"connections.h"
#include"../log/logger.h"


Server::Server(int p){
    port = p;
}
void Server::start(){
    Socket listener(port);
    listener.create();
    listener.bind_socket();
    listener.listen_socket();
    Logger::log(INFO, "Server started on port "+ std::to_string(port));

    fcntl(listener.fd, F_SETFL, O_NONBLOCK);

    int epfd = epoll_create1(0);
    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = listener.fd;

    epoll_ctl(epfd, EPOLL_CTL_ADD, listener.fd, &ev);

    epoll_event events[1024];
    unordered_map<int, Connections*>connections;


    while(true){
        int n = epoll_wait(epfd, events, 1024, -1);
        for(int i = 0; i < n; i++){
            int fd = events[i].data.fd;
            //handle the listner to add more connections 
            if(fd == listener.fd){
                while(true){
                    int client_fd = listener.accept_con();
                    if(client_fd < 0){
                        break;
                    }
                    Logger::log(INFO, "New connection fd=" + std::to_string(client_fd));

                    fcntl(client_fd, F_SETFL, O_NONBLOCK);
                    Connections * obj = new Connections();
                    obj->fd = client_fd;
                    connections[client_fd] = obj;
                    epoll_event client_ev;
                    client_ev.events = EPOLLIN|EPOLLERR | EPOLLHUP;
                    client_ev.data.fd = client_fd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &client_ev);
                }
            }
            //handle the read or write event of a connection 
            else{ 
                if(events[i].events & (EPOLLERR | EPOLLHUP)){
                    close(fd);
                    delete connections[fd];
                    connections.erase(fd);
                    continue;
                }

                auto &conn = connections[fd];


                if(events[i].events & EPOLLIN){
                    conn->handle_read();
                }
                if(events[i].events & EPOLLOUT){
                    conn->handle_write();
                }

                if(conn->state == Connections::CLOSED){
                    close(fd);
                    delete connections[fd];
                    connections.erase(fd);
                    continue;
                }
                epoll_event ev;
                ev.data.fd = fd;

                if(connections[fd]->state == Connections::WRITING){
                    ev.events = EPOLLOUT| EPOLLERR| EPOLLHUP;
                }
                else{
                    ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;
                    
                }
                epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
            }


        }
        
    }

}

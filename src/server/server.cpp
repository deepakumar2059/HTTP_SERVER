#include"server.h"
#include"../http/handler.h"
#include"../network/socket.h"
#include<sys/socket.h>
#include<iostream>
#include<unistd.h>
#include<thread>


Server::Server(int p){
    port = p;
}
void Server::start(){
    Socket listener(port);
    listener.create();
    listener.bind_socket();
    listener.listen_socket();
    std::cout<<"server starts listening on "<<port<<std::endl;
    while(true){
        int client_fd = listener.accept_con();
        std::thread t(handle_client, client_fd);
        t.detach();
    }

}






/*
while(true){
    int client_fd = accept();
    string buffer;
    while(true){
        char temp[1024];
        int n = recv(client_fd, temp, 1024, 0);
        if( n <= 0){
            break;
        }
        
        buffer.append(buffer, n);
        while(true){
            if(!request_complete(buffer)){
                break;
            }
            request = extract(buffer);
            response = process(request);
            sendAll(resonse);
        }
    }
}
*/



/*
void sendAll(){
    int len = data.size();
    int totalSent = 0;
    while(totalSent < len){
        int n = send(fd, data + totalSent, len -totalSent, 0);
        if(n <= 0){
            break;
        }
        totalSent += n;
    }
}
*/
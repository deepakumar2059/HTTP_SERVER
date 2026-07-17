#include<iostream>
#include<thread>
#include<sys/socket.h>
#include<sys/epoll.h>
using namespace std;
int main(){
    int epfd = epoll_create1(0);
    int fd = socket(AF_INET, SOCK_STREAM, 0 );
    cout<<fd<<endl;
    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    ev.events = EPOLLOUT;
    if(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1){
        perror("epoll_error");
    }
    cout<<"returning"<<endl;
}

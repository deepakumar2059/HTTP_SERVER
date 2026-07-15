#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<iostream>
#include"socket.h"

Socket::Socket(int p){
    port = p;
}

void Socket::create(){
    fd = socket(AF_INET, SOCK_STREAM, 0);
}

void Socket::bind_socket(){
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(fd, (struct sockaddr*)&addr, sizeof(addr));
}

void Socket::listen_socket(){
    listen(fd, 10);
}

int Socket::accept_con(){
    return accept(fd, NULL, NULL);
}



Socket::~Socket(){
    if(fd != -1){
        close(fd);
    }
}
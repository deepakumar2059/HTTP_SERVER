#include<sys/socket.h>
#include"handler.h"
#include"httpParser.h"

void handle_client(int client_fd){
    Buffer buffer;
    char temp[1024];
    Parser parser;
    while(true){
        int n = recv(client_fd, temp, 1024, 0);
        if(n <= 0){
            close(client_fd);
            return ;
        }
        buffer.append(temp, n);
        while(parser.consume(buffer) == Parser::COMPLETE){
            Request request = parser.getRequest();

            cout<<request.method<<endl;
            cout<<request.resource<<endl;
            cout<<request.version<<endl;
            for(auto &a: request.headers){
                cout<<a.first<<" : "<<a.second<<endl;
            }

            string response =
                "HTTP/1.1 200 OK\r\n"
                "Connection: keep-alive\r\n"
                "\r\n";
            send(client_fd, response.c_str(), response.size(), 0);
            parser.reset();
        }
    }
}
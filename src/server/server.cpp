#include"server.h"
#include"../http/httpParser.h"
#include"../network/socket.h"
#include<sys/socket.h>
#include<iostream>
#include<unistd.h>
#include<sys/time.h>


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
        timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        cout<<"connection number : "<<client_fd<<endl;

        Buffer buffer;
        Parser parser;
        char temp[1024];
        while(true){
            int n = recv(client_fd, temp, 1024, 0);
            if(n <=0){
                break;
            }
            buffer.append(temp, n);
            if(parser.consume(buffer) == Parser::COMPLETE){
                
                Request request = parser.getRequest();

                cout<<request.method<<endl;
                cout<<request.resource<<endl;
                cout<<request.version<<endl;
                
                for(auto &h:request.headers){
                    cout<<h.first<<" : "<<h.second<<endl;
                }

                cout<<request.body<<endl;

                string response =
                    "HTTP/1.1 200 OK\r\n"
                    "Connection: close\r\n"
                    "\r\n";
                
                send(client_fd, response.c_str(), response.size(), 0);
                // handle(request);
                parser.reset();
                break;
            } 
            
        }
        close(client_fd);
            
          
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
#include"connections.h"
#include<sys/socket.h>
void Connections::handle_read(){
        char temp[1024];
        int n = recv(fd, temp, 1024, 0);

        if(n == 0){
            state = CLOSED;
            return;
        }
        if(n < 0){
            if(errno == EAGAIN || errno == EWOULDBLOCK){
                return ;
            }
            state = CLOSED;
            return ;
        }
        buffer.append(temp, n);
        while(parser.consume(buffer) == Parser::COMPLETE){
            Request request = parser.getRequest();
            parser.reset();
            std::cout<<request.method<<endl;
            std::cout<<request.resource<<endl;
            //process
            string respone = 
                "HTTP/1.1 200 OK\r\n"
                "Content-Length: 11\r\n"
                "Connection: keep-alive\r\n"
                "\r\n"
                "hello world\r\n";
            write_queue.push(respone);
        }
        if(!write_queue.empty()){
            write_buffer = write_queue.front();
            write_queue.pop();
            len = write_buffer.size();
            totalSend = 0;
            state = WRITING;
        }
}
void Connections::handle_write(){
        int n = send(fd, write_buffer.c_str() + totalSend, len - totalSend, 0 );
        if(n < 0){
            if(errno == EAGAIN || errno == EWOULDBLOCK){
                return;
            }
            state = CLOSED;
            return ;
        }
        totalSend += n;
    if(totalSend == len){
        if(!write_queue.empty()){
            write_buffer = write_queue.front();
            write_queue.pop();
            len = write_buffer.size();
            totalSend = 0;
        }
        else{
            state = READING;
        }
        
    }
   
}
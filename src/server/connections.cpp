#include"connections.h"
#include<sys/socket.h>
#include<sys/sendfile.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<unistd.h>


/* 
    from this comment to the next comment 
    all these three functions ends_with() , getMIMEtypes() and
    handler_response_builder() should be separated to different class or 
    functions or worker thread
*/

bool ends_with(const std::string &file, const std::string &suffix){
    if(file.size() < suffix.size()) return false;
    return file.compare(file.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string getMIMEtypes(const std::string &file){
    if(ends_with(file, ".html")) return "text/html";
    if(ends_with(file, ".css")) return "text/css";
    if(ends_with(file, ".js")) return "text/javascript";
    if(ends_with(file, ".json")) return "application/json";
    if(ends_with(file, ".png")) return "image/png";
    return "text/plain";
}

void Connections::handler_response_builder(Request & request){
    Response response;

    //basic router for POST method
    // if(request.method == "POST"){    
    //     std::cout<<request.body<<endl;
    //     int file_fd = open("new.txt", O_RDWR);
    //     write(file_fd, request.body.c_str(), request.body.size());
    //     close(file_fd);
    //     response.offset = 0;
    //     response.size = 0;
    //     response.state = Response::SENDING_HEADER;
    //     response.file_fd = -1;
    //     response.header_sent = 0;
    //     response.header=
    //         "HTTP/1.1 201 Created"
    //         "Content-Length: 0\r\n"
    //         "Connection: keep-alive\r\n"
    //         "\r\n";
    //     response_queue.push(response);
    //     return;
    // }


    string resource = request.resource;
    if(resource == "/"){
        resource = "/index.html";
    }
   
    // if(resource.size() > 0 && resource[0] == '/'){
    //     resource.erase(0, 1);
    // }

    string mimeType = getMIMEtypes(resource);

    string filename = "../static_files" + resource;
    response.file_fd = open(filename.c_str(), O_RDONLY);
    if(response.file_fd < 0){
        response.header =
            "HTTP/1.1 404 NOT FOUND\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 0\r\n"
            "Connection: keep-alive\r\n\r\n";
        response.size = 0;
        response.offset = 0;
        response.header_sent = 0;
        response.state = Response::SENDING_HEADER;
        response_queue.push(response);
        return;
    }

    struct stat st;
    if(fstat(response.file_fd, &st) < 0){
        close(response.file_fd);
        response.file_fd = -1;
        response.header =
            "HTTP/1.1 500 INTERNAL SERVER ERROR\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 0\r\n"
            "Connection: keep-alive\r\n\r\n";
        response.size = 0;
        response.offset = 0;
        response.header_sent = 0;
        response.state = Response::SENDING_HEADER;
        response_queue.push(response);
        return;
    }

    response.size = st.st_size;
    response.offset = 0;
    response.header = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: " + std::to_string(st.st_size) + "\r\n"
        "Content-Type: " + mimeType + "\r\n"
        "Connection: keep-alive\r\n\r\n";
    response.header_sent = 0;
    response.state = Response::SENDING_HEADER;
    response_queue.push(response);
}

/* 
    only the part below this comment will be included in the connections class 
    for time constraint and simplicity requestHandler() and responseBuilder() 
    are integrated into this class
*/


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
            std::cout<<request.method<<"\n";
            std::cout<<request.resource<<"\n";
            handler_response_builder(request);
        }
        if(!response_queue.empty()){
            state = WRITING;
        }
}
void Connections::handle_write(){
    while(!response_queue.empty()){
        Response & res = response_queue.front();

        if(res.state == Response::SENDING_HEADER){
            int n = send(fd, res.header.data()+ res.header_sent, res.header.size() - res.header_sent, 0 );
            if(n > 0){
                res.header_sent += n;
            }
            else if(n < 0){
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    return;
                }
                state = CLOSED;
                return;
            }
            if(res.header_sent == res.header.size()){
                if(res.file_fd < 0 || res.size == 0){
                    res.state = Response::DONE;
                } else {
                    res.state = Response::SENDING_FILE;
                }
            }
        }

        if(res.state == Response::SENDING_FILE){
            int n = sendfile(fd, res.file_fd, &res.offset, res.size - res.offset);
            if(n < 0){
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    return;
                }
                state = CLOSED;
                return;
            }
            if(res.offset == res.size){
                close(res.file_fd);
                res.state = Response::DONE;
            }
        }
        if(res.state == Response::DONE){
            response_queue.pop();
        }
    }
    state = READING;
}







/*

*/
#include"../http/httpParser.h"
#include<queue>

struct Response{
    int file_fd;
    off_t offset;
    off_t size;
    std::string header;
    size_t header_sent;
    enum{
        SENDING_HEADER,
        SENDING_FILE,
        DONE
    } state;

};

class Connections{
    public:
    int fd;
    Parser parser;
    Buffer buffer;
    string write_buffer;
    queue<Response>response_queue;
    void handle_read();
    void handle_write();
    void handler_response_builder(Request & request);
    enum State{
        READING, WRITING, CLOSED
    };
    State state;

};

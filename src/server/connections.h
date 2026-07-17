#include"../http/httpParser.h"
#include<queue>
class Connections{
    public:
    int fd;
    Parser parser;
    Buffer buffer;
    string write_buffer;
    queue<string>write_queue;
    int len;
    int totalSend ;
    void handle_read();
    void handle_write();
    enum State{
        READING, WRITING, CLOSED
    };
    State state;

};

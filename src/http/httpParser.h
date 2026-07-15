#include<bits/stdc++.h>
using namespace std;
class Request{
    public:
    string method;
    string resource;
    string version;
    unordered_map<string, string>headers;
    string body;
    void reset();


};
class Buffer{
    private:
    string data;
    public:
    void append(char s[], int n);
    bool hasLine();
    string getLine();
    bool hasBytes(int n);
    string getBytes(int n);

};


class Parser{
    public:
    enum State{REQUEST_LINE, HEADERS, BODY, COMPLETE };
    private:
    State state ;
    Request request;
    int content_length;
    public:
    Parser();
    State consume(Buffer &buffer);
    Request getRequest();
    void reset();
};


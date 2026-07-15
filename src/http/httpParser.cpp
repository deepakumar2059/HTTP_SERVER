#include"httpParser.h"

void Request::reset(){
    method.clear();
    resource.clear();
    version.clear();
    headers.clear();
    body.clear();
}

Parser::Parser(){
    state = REQUEST_LINE;
    content_length = 0; 
}

Parser::State Parser::consume(Buffer &buffer){
    while(true){
        if(state == REQUEST_LINE){
            if(!buffer.hasLine()) return state;
            string line = buffer.getLine();
            stringstream ss(line);
            ss>>request.method>>request.resource>>request.version;
            state = HEADERS;
        }

        else if(state == HEADERS){
            if(!buffer.hasLine()) return state;

            string line = buffer.getLine();
            if(line.size() == 0){
                if(request.headers.count("Content-Length")){
                    content_length = stoi(request.headers["Content-Length"]);
                    state = BODY;
                }
                else{
                    state = COMPLETE;
                    return state;
                }
                continue;
            }
            int pos = line.find(": ");
            if(pos != string::npos){
                string key = line.substr(0, pos);
                string value = line.substr(pos+2);
                request.headers[key] = value;
            }
        }
        else if(state == BODY){
            if(!buffer.hasBytes(content_length)) return state;
            request.body = buffer.getBytes(content_length);
            state = COMPLETE;
            return state;
        }
        else{
            return state;
        }
    }
    return state;
}
Request Parser::getRequest(){
    return request;
}

void Parser::reset(){
    state = REQUEST_LINE;
    request.reset();
    content_length = 0;
}






void Buffer::append(char s[], int n){
    data.append(s, n);
}

bool Buffer::hasLine(){
    return data.find("\r\n") != string::npos;
}
    
string Buffer::getLine(){
    int pos = data.find("\r\n");
    string line = data.substr(0,pos);
    data.erase(0, pos+2);
    return line;
}

bool Buffer::hasBytes(int n){
    return data.size() >= n;
}

string Buffer::getBytes(int n){
    string bytes = data.substr(0,n);
    data.erase(0, n);
    return bytes;
}




   
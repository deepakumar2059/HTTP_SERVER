#pragma once
#include<iostream>
#include<ctime>
#include<cstring>
enum LogLevel{
    INFO,
    ERROR,
    DEBUG

};

class Logger{
    private:
    static std::string get_time(){
        time_t now = time(0);
        char *dt = ctime(&now);
        dt[strlen(dt)-1] = '\0';
        return std::string(dt);
    }
    public:
    static void log(LogLevel level, const std::string & msg){
        std::string level_str;
        switch (level){
            case INFO: level_str = "INFO"; break;
            case ERROR: level_str = "ERROR"; break; 
            case DEBUG: level_str = "DEBUG"; break;
        
        }
        std::cout<< "["<< Logger::get_time()<<"] "
                    << "[" << level_str << "] "
                    << msg <<std::endl;
    }

};
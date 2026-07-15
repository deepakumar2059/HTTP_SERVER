#include<iostream>
#include<thread>
using namespace std;
void func(){
    cout<<"thread started\n";
    for(int i = 0; i < 5; i++){
        cout<<i<<endl;
    }
    cout<<"thread finished\n";
}

int main(){
    thread t(func);
    cout<<"main is waiting ---\n";
    
    int i = 50000;
    while(i)i--;
    cout<<"main thread continuous\n";
}
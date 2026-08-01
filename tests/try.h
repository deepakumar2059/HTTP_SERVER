#include<bits/stdc++.h>
using namespace std;
int main(){
    time_t now = time(0);
    cout<<now<<endl;
    char *dt = ctime(&now);
    cout<<dt<<endl;
    dt[strlen(dt)-1] = '\0';
    cout<<dt<<endl;
}
#include"threadPool.h"
#include"handler.h"



void ThreadPool::func(){
    while(true){
        std::unique_lock<std::mutex> lock(mtx);

        cv.wait(lock, [this]{
            return stop || !tasks.empty();
        });

        if(stop && tasks.empty()){
            return;
        }
        int client_fd = tasks.front();
        tasks.pop();
        lock.unlock();
        handle_client(client_fd);

    }
   
}


ThreadPool::ThreadPool(int n){
    stop = false;
    for(int i = 0; i < n; i++){
        
        workers.emplace_back(&ThreadPool::func, this);
    }

}

void ThreadPool::enqueue(int client_id){
    {
        std::lock_guard<std::mutex> lock(mtx);
        tasks.push(client_id);

    }
    cv.notify_one();
}

ThreadPool:: ~ThreadPool(){
    {
        std::lock_guard<std::mutex> lock(mtx);
        stop = true;
    }
    cv.notify_all();

    for(auto &t: workers){
        if(t.joinable()){
            t.join();
        }
    }

}
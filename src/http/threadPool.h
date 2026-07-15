#include<queue>
#include<thread>
#include<vector>
#include<mutex>
#include<condition_variable>

class ThreadPool{
    private:
    std::queue<int>tasks;
    std::vector<std::thread>workers;
    std::mutex mtx;
    std::condition_variable cv;
    bool stop;

    public:
    ThreadPool(int);
    void enqueue(int);
    void func();
    ~ThreadPool();
};
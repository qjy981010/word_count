#ifndef BLOCKQUEUE_H_
#define BLOCKQUEUE_H_

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

class ThreadQueue
{
public:
    ThreadQueue() = default;
    ThreadQueue( const ThreadQueue & ) = delete;
    ThreadQueue &operator=( const ThreadQueue & ) = delete;

private:
    std::queue<char*>        queue_;
    mutable std::mutex       mutex_;
    std::condition_variable  cond_;
    
public:
    void pop( char** elem );
    bool empty() const;
    void push( char* elem );
    char* front() const;
    void justgo();
};

#endif
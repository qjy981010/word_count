#include "blockqueue.h"

void ThreadQueue::pop(char** elem) {
    std::unique_lock<std::mutex> lock( mutex_ );
    cond_.wait(lock, [this]() { return !queue_.empty(); }); // block
    *elem = queue_.front();
    queue_.pop();
}

bool ThreadQueue::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}

void ThreadQueue::push( char* elem ) {
   	std::lock_guard<std::mutex> lock(mutex_);
   	queue_.push(elem);
    cond_.notify_one();
}

char* ThreadQueue::front() const {
    std::lock_guard<std::mutex> lock( mutex_ );
    return queue_.front();
}

void ThreadQueue::justgo() {
	cond_.notify_all();	
}

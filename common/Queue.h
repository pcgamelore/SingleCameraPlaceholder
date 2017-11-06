#ifndef CONCURRENT_QUEUE_
#define CONCURRENT_QUEUE_

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <stdio.h>

template <typename T>
class Queue
{
 public:

  T pop() 
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    while (queue_.empty())
    {
//printf("waiting for cond mutex\n");
      cond_.wait(mlock);
    }
    auto val = queue_.front();
    queue_.pop();
    return val;
  }

  void pop(T& item)
  {
//printf("Queue pop function\n");
    std::unique_lock<std::mutex> mlock(mutex_);
    while (queue_.empty())
    {
//printf("waiting for cond mutex\n");
      cond_.wait(mlock);
    }
    item = queue_.front();
    queue_.pop();
  }

  void push(const T& item)
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_.push(item);
    mlock.unlock();
    cond_.notify_one();
  }
  bool isempty()
  {
    return queue_.empty();
  } 
  int getsize()
  {
    int size;
    std::unique_lock<std::mutex> mlock(mutex_);
    size = queue_.size();
    mlock.unlock();
    return size;
  } 
  Queue()=default;
  Queue(const Queue&) = delete;            // disable copying
  Queue& operator=(const Queue&) = delete; // disable assignment
  
 private:
  std::queue<T> queue_;
  std::mutex mutex_;
  std::condition_variable cond_;
};

#endif

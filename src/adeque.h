#ifndef ADEQUE_H
#define ADEQUE_H

#include <deque>
#include <mutex>
#include <condition_variable>

struct Coroutine;

class AsyncDeque {
public:
    Coroutine *pop_front();
    Coroutine *pop_back();
    void push_front(Coroutine *coroutine);
    void push_back(Coroutine *coroutine);
    int size();

private:
    std::mutex mu_;
    std::condition_variable cv_;
    std::deque<Coroutine *> deque_;
};

#endif
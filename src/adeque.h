#ifndef ADEQUE_H
#define ADEQUE_H

#include <deque>
#include <mutex>
#include <condition_variable>

class AsyncDeque {
public:
private:
    std::mutex mu_;
    std::condition_variable cv_;
};

#endif
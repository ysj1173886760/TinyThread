#include "adeque.h"

Coroutine *AsyncDeque::pop_front() {
    std::unique_lock<std::mutex> guard(mu_);
    if (deque_.empty()) {
        cv_.wait(guard, [&]() {
            return !deque_.empty();
        });
    }

    Coroutine *res = deque_.front();
    deque_.pop_front();
    return res;
}

Coroutine *AsyncDeque::pop_back() {
    std::unique_lock<std::mutex> guard(mu_);
    if (deque_.empty()) {
        cv_.wait(guard, [&]() {
            return !deque_.empty();
        });
    }

    Coroutine *res = deque_.back();
    deque_.pop_back();
    return res;
}

void AsyncDeque::push_front(Coroutine *coroutine) {
    std::lock_guard<std::mutex> guard(mu_);
    deque_.push_front(coroutine);
    cv_.notify_one();
}

void AsyncDeque::pop_front(Coroutine *coroutine) {
    std::lock_guard<std::mutex> guard(mu_);
    deque_.push_back(coroutine);
    cv_.notify_one();
}
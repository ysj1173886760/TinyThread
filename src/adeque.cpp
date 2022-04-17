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

Coroutine *AsyncDeque::pop_back_no_block() {
    std::unique_lock<std::mutex> guard(mu_);
    if (deque_.empty()) {
        return nullptr;
    }

    Coroutine *res = deque_.back();
    deque_.pop_back();
    return res;
}

Coroutine *AsyncDeque::pop_front_no_block() {
    std::unique_lock<std::mutex> guard(mu_);
    if (deque_.empty()) {
        return nullptr;
    }

    Coroutine *res = deque_.front();
    deque_.pop_front();
    return res;
}

void AsyncDeque::push_front(Coroutine *coroutine) {
    std::unique_lock<std::mutex> guard(mu_);
    deque_.push_front(coroutine);
    cv_.notify_one();
}

void AsyncDeque::push_back(Coroutine *coroutine) {
    std::unique_lock<std::mutex> guard(mu_);
    deque_.push_back(coroutine);
    cv_.notify_one();
}

int AsyncDeque::size() {
    std::unique_lock<std::mutex> guard(mu_);
    return deque_.size();
}
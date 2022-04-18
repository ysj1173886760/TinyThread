#ifndef MASTER_H
#define MASTER_H

#include "coroutine.h"
#include "logger.h"
#include <vector>
#include <thread>
#include <unordered_map>
#include <atomic>
#include <condition_variable>
#include <map>

class Master {
public:
    void work();
    static Master& getInstance() {
        static Master m(2);
        return m;
    }

    int worker_num_;
    int counter_;
    std::unordered_map<std::thread::id, Scheduler*> table_;
    std::mutex mu_;
    std::atomic<bool> initialized_;

private:

    Master(int worker_num) {
        worker_num_ = worker_num;
        initialized_.store(false);
        for (int i = 0; i < worker_num; i++) {
            std::thread(&Master::work, this).detach();
        }
    }
public:
    Master(Master const &) = delete;
    void operator=(Master const &) = delete;

};

class WaitGroup {
public:
    WaitGroup(int worker): counter_(0), worker_(worker) {}
    void done() {
        counter_.fetch_add(1);
    }
    void wait() {
        while (counter_.load() < worker_) {}
    }
    bool try_wait() {
        return counter_.load() == worker_;
    }

private:
    std::atomic<int> counter_;
    int worker_;
};

Master master();

void create(user_func func, void *args);
void initialize();
void print_current_scheduler();
void check_stack();

#endif
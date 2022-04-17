#ifndef MASTER_H
#define MASTER_H

#include "coroutine.h"
#include "logger.h"
#include <vector>
#include <thread>
#include <unordered_map>
#include <atomic>

class Master {
public:
    void work();
    static Master& getInstance() {
        static Master m(1);
        return m;
    }

    int worker_num_;
    std::atomic<int> barrier_;
    std::unordered_map<std::thread::id, Scheduler*> table_;

private:

    Master(int worker_num) {
        worker_num_ = worker_num;
        barrier_.store(0);
        for (int i = 0; i < worker_num; i++) {
            std::thread(&Master::work, this).detach();
        }
        table_.clear();
    }
public:
    Master(Master const &) = delete;
    void operator=(Master const &) = delete;


};

Master master();

void create(user_func func, void *args);
void initialize();

#endif
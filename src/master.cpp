#include "master.h"
#include "logger.h"

void Master::work() {
    // first initialize the mapping from thread_id to scheduler
    auto id = std::this_thread::get_id();
    table_[id] = new Scheduler;
    barrier_.fetch_add(1);

    LOG_INFO("barrier %d", barrier_.load());
    // synchronize here
    while (barrier_.load() != worker_num_) {}

    // start working
    auto s = table_[id];


    while (true) {
        // first try to pop the task locally
        auto ct = s->deque_.pop_front_no_block();
        if (ct != nullptr) {
            s->resume(ct);
            continue;
        }

        // then try to steal tasks
        for (const auto &[key, value] : table_) {
            if (key == id) {
                continue;
            }
            auto work = value->deque_.pop_back_no_block();
            if (work != nullptr) {
                value->resume(work);
                break;
            }
        }

    }

}

void create(user_func func, void *args) {
    auto id = std::this_thread::get_id();
    Scheduler *s;
    bool is_master = false;
    if (!Master::getInstance().table_.count(id)) {
        // create task inside worker thread
        is_master = true;
        LOG_INFO("master table size %d", Master::getInstance().table_.size());
        s = Master::getInstance().table_.begin()->second;
    } else {
        // create task in main thread
        s = Master::getInstance().table_[id];
    }
    LOG_INFO("create thread %d", is_master);

    for (const auto &[key, value] : Master::getInstance().table_) {
        LOG_INFO("%d", key);
    }
    s->spawn(func, args, is_master);
}

void initialize() {
    while (Master::getInstance().barrier_.load() != Master::getInstance().worker_num_) {}
}
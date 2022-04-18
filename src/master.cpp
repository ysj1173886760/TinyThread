#include "master.h"
#include "logger.h"
#include "defs.h"

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
                auto arg = (Arg *)(work->user_args_);
                LOG_INFO("%p steal work from %p left %d right %d", id, key, arg->left, arg->right);
                s->resume(work);
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
        s = Master::getInstance().table_.begin()->second;
        LOG_INFO("spawn thread %p", Master::getInstance().table_.begin()->first);
    } else {
        // create task in main thread
        s = Master::getInstance().table_[id];
        LOG_INFO("spawn thread %p", id);
    }

    s->spawn(func, args, is_master);
}

void initialize() {
    while (Master::getInstance().barrier_.load() != Master::getInstance().worker_num_) {}
}

void print_current_scheduler() {
    auto id = std::this_thread::get_id();
    if (!Master::getInstance().table_.count(id)) {
        LOG_INFO("not in worker thread");
    } else {
        LOG_INFO("current scheduler %p", Master::getInstance().table_[id]);
    }
}

void check_stack() {
    auto id = std::this_thread::get_id();
    char dummy = 0;
    if (!Master::getInstance().table_.count(id)) {
        LOG_INFO("not in worker thread");
    } else {
        LOG_INFO("%p stack size %d", id,
            Master::getInstance().table_[id]->stack_ + stack_size - &dummy);
    }
}
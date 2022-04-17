#ifndef COROUTINE_H
#define COROUTINE_H

#include <ucontext.h>
#include <vector>
#include <atomic>
#include "adeque.h"

typedef void (*user_func)(void *);

// 1MB stack
const int stack_size = 1024 * 1024;

class Scheduler;
class AsyncDeque;

enum State {
    Dead = 0,
    Ready = 1,
    Running = 2,
    SUSPEND = 3,
};

struct Coroutine {
    user_func user_func_;
    void *user_args_;
    ucontext_t ctx_;
    Scheduler *sched_;
    State state_;
    char *stack_;
    int stack_cap_;
    int stack_size_;

    Coroutine(): user_func_(nullptr),
                 user_args_(nullptr),
                 sched_(nullptr),
                 state_(Dead),
                 stack_(nullptr),
                 stack_cap_(0),
                 stack_size_(0) {}

};

Coroutine *create_coroutine(user_func func, void *args);

class Scheduler {
public:
    Scheduler(): running_(nullptr) {}
    void spawn(user_func func, void *args, bool is_master);
    void resume(Coroutine *coroutine);

private:
    void save_stack(Coroutine *coroutine);

// member variables
public:
    char stack_[stack_size];
    ucontext_t main_;
    AsyncDeque deque_;
    Coroutine *running_;
};


#endif
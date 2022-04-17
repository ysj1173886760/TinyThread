#ifndef COROUTINE_H
#define COROUTINE_H

#include <ucontext.h>
#include <vector>
#include "adeque.h"

// 1MB stack
const int stack_size = 1024 * 1024;

class Scheduler;

enum State {
    Dead = 0,
    Ready = 1,
    Running = 2,
    SUSPEND = 3,
};

struct Coroutine {
    void *user_func_;
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

class Scheduler {
public:
    Scheduler(): running_(nullptr) {}
    void spawn(void *func, void *args);

private:
    void resume(Coroutine *coroutine);
    void yield();
    void save_stack(Coroutine *coroutine);

// member variables
private:
    char stack_[stack_size];
    ucontext_t main_;
    AsyncDeque deque_;
    Coroutine *running_;
};


#endif
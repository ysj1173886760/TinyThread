#ifndef COROUTINE_H
#define COROUTINE_H

#include <ucontext.h>
#include <vector>

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
};

class Scheduler {
public:
    Scheduler() {}

private:
    char stack_[stack_size];
    ucontext_t main_;
};


#endif
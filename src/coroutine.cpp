#include "coroutine.h"
#include <assert.h>
#include <cstring>

void Scheduler::spawn(user_func func, void *args) {
    Coroutine *ct = new Coroutine;
    getcontext(&ct->ctx_);
    ct->ctx_.uc_stack.ss_sp = stack_;
    ct->ctx_.uc_stack.ss_size = stack_size;
    // since we may not resume at current thread/scheduler
    // so uc_link is not useful
    // instead, we shall use setcontext directly
    ct->ctx_.uc_link = 0;
    ct->user_func_ = func;
    ct->user_args_ = args;
    ct->sched_ = this;

    Coroutine *cur = running_;
    assert(cur != nullptr);

    makecontext(&ct->ctx_, (void (*)())(wrapper), 2, cur, ct);

    // save the stack
    save_stack(cur);
    running_ = nullptr;

    // switch to new func
    swapcontext(&cur->ctx_, &ct->ctx_);
}

void wrapper(Coroutine *pre, Coroutine *cur) {
    // they should running on the same worker
    assert(pre->sched_ == cur->sched_);

    auto s = cur->sched_;
    s->deque_.push_back(pre);

    s->running_ = cur;
    // call user defined func
    cur->user_func_(cur->user_args_);
    
    // reacquire the scheduler
    s = cur->sched_;
    assert(s->running_ == cur);

    // then we delete the coroutine
    // and switch back to scheduler
    setcontext(&s->main_);
}

void Scheduler::resume(Coroutine *coroutine) {
    coroutine->sched_ = this;
    memcpy(stack_ + stack_size - coroutine->stack_size_,
           coroutine->stack_, coroutine->stack_size_);
    running_ = coroutine;
    swapcontext(&main_, &coroutine->ctx_);
}

void Scheduler::save_stack(Coroutine *coroutine) {
    char dummy = 0;
    char *top = stack_ + stack_size;

    if (coroutine->stack_cap_ < top - &dummy) {
        delete(coroutine->stack_);
        coroutine->stack_cap_ = top - &dummy;
        coroutine->stack_ = new char[top - &dummy];
    }

    coroutine->stack_size_ = top - &dummy;
    memcpy(coroutine->stack_, &dummy, coroutine->stack_size_);
}
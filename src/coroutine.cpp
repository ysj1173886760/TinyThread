#include "coroutine.h"
#include <assert.h>
#include <cstring>

void wrapper(Coroutine *pre, Coroutine *cur) {
    // they should running on the same worker
    auto s = cur->sched_;
    if (pre != nullptr) {
        assert(pre->sched_ == cur->sched_);
        s->deque_.push_back(pre);
    }

    s->running_ = cur;
    // call user defined func
    cur->user_func_(cur->user_args_);
    
    // reacquire the scheduler
    s = cur->sched_;
    assert(s->running_ == cur);

    delete cur;

    s->running_ = nullptr;

    // then we delete the coroutine
    // and switch back to scheduler
    setcontext(&s->main_);
}

void Scheduler::spawn(user_func func, void *args, bool is_master) {
    Coroutine *cur = new Coroutine;
    getcontext(&cur->ctx_);
    cur->ctx_.uc_stack.ss_sp = stack_;
    cur->ctx_.uc_stack.ss_size = stack_size;
    // since we may not resume at current thread/scheduler
    // so uc_link is not useful
    // instead, we shall use setcontext directly
    cur->ctx_.uc_link = 0;
    cur->user_func_ = func;
    cur->user_args_ = args;
    cur->sched_ = this;

    if (is_master) {
        makecontext(&cur->ctx_, (void (*)())(wrapper), 2, nullptr, cur);
        deque_.push_back(cur);
        return;
    }

    Coroutine *pre = running_;
    assert(pre != nullptr);

    makecontext(&cur->ctx_, (void (*)())(wrapper), 2, pre, cur);

    // save the stack
    save_stack(pre);
    running_ = nullptr;

    // switch to new func
    swapcontext(&pre->ctx_, &cur->ctx_);
}

void Scheduler::resume(Coroutine *coroutine) {
    coroutine->sched_ = this;
    if (coroutine->stack_) {
        memcpy(stack_ + stack_size - coroutine->stack_size_,
            coroutine->stack_, coroutine->stack_size_);
    }
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
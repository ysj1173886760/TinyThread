#include "coroutine.h"
#include <assert.h>
#include <cstring>

void Scheduler::spawn(void *func, void *args) {
    Coroutine *ct = new Coroutine;
    getcontext(&ct->ctx_);
    ct->ctx_.uc_stack.ss_sp = stack_;
    ct->ctx_.uc_stack.ss_size = stack_size;
    // since we may not resume at current thread/scheduler
    // so uc_link is not useful
    // instead, we shall use setcontext directly
    ct->ctx_.uc_link = 0;

    makecontext(&ct->ctx_, (void (*)())(wrapper), 0);

    Coroutine *cur = running_;
    assert(cur != nullptr);

    swapcontext(&cur->ctx_, &ct->ctx_);
}

void wrapper() {

}

void Scheduler::resume(Coroutine *coroutine) {
    coroutine->sched_ = this;
    memcpy(stack_ + stack_size - coroutine->stack_size_,
           coroutine->stack_, coroutine->stack_size_);
    running_ = coroutine;
    swapcontext(&main_, &coroutine->ctx_);
}

void Scheduler::yield() {
    Coroutine *ct = running_;
    save_stack(ct);
    running_ = nullptr;
    swapcontext(&ct->ctx_, &main_);
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
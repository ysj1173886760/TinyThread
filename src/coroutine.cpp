#include "coroutine.h"
#include "logger.h"
#include "defs.h"
#include <assert.h>
#include <cstring>
#include <thread>

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

    // delete cur;

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

    auto arg = (Arg *)args;
    LOG_INFO("spawn working thread %p left %d right %d", std::this_thread::get_id(), arg->left, arg->right);
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
    Scheduler *pre = coroutine->sched_;
    coroutine->sched_ = this;
    if (coroutine->stack_) {
        assert(pre != nullptr);
        memcpy(stack_ + stack_size - coroutine->stack_size_,
            coroutine->stack_, coroutine->stack_size_);
        coroutine->ctx_.uc_mcontext.gregs[REG_RIP] += stack_ - pre->stack_;
        coroutine->ctx_.uc_mcontext.gregs[REG_RSP] += stack_ - pre->stack_;
        coroutine->ctx_.uc_mcontext.gregs[REG_RBP] += stack_ - pre->stack_;
    } else {
        // first time running
        // re-construct the context
        coroutine->ctx_.uc_stack.ss_sp = stack_;
        coroutine->ctx_.uc_stack.ss_size = stack_size;
        makecontext(&coroutine->ctx_, (void (*)())(wrapper), 2, nullptr, coroutine);
    }
    auto arg = (Arg *)coroutine->user_args_;
    LOG_INFO("%p resume thread left %d right %d", std::this_thread::get_id(), arg->left, arg->right);
    running_ = coroutine;
    swapcontext(&main_, &coroutine->ctx_);
}

void Scheduler::save_stack(Coroutine *coroutine) {
    char dummy = 0;
    char *top = stack_ + stack_size;

    if (coroutine->stack_cap_ < top - &dummy) {
        // delete[] coroutine->stack_;
        coroutine->stack_cap_ = top - &dummy;
        coroutine->stack_ = new char[top - &dummy];
    }

    coroutine->stack_size_ = top - &dummy;
    LOG_INFO("cur stack size %d total stack size %d", coroutine->stack_size_, stack_size);
    assert(coroutine->stack_size_ < stack_size);
    LOG_INFO("%p %p %p", coroutine->stack_, &dummy, coroutine->stack_size_);
    memcpy(coroutine->stack_, &dummy, coroutine->stack_size_);
}
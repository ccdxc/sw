//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// thread library
//------------------------------------------------------------------------------

#ifndef __SDK_THREAD_HPP__
#define __SDK_THREAD_HPP__

#include <pthread.h>
#include "include/sdk/base.hpp"
#include "include/sdk/timestamp.hpp"
#include "lib/lfq/lfq.hpp"

namespace sdk {
namespace lib {

typedef enum thread_role_e {
    THREAD_ROLE_CONTROL,
    THREAD_ROLE_DATA
} thread_role_t;

//------------------------------------------------------------------------------
// thread entry function
//------------------------------------------------------------------------------
typedef void *(*thread_entry_func_t)(void *ctxt);

#define SDK_MAX_THREAD_NAME_LEN        30

// call one of the below two macros from the entry function of every thread
#define SDK_THREAD_INIT(ctxt)                                          \
{                                                                      \
    int __rv__, __old_type__;                                          \
    __rv__ = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,        \
            &__old_type__);                                            \
    SDK_ABORT(__rv__ == 0);                                            \
    sdk::lib::thread::set_current_thread((sdk::lib::thread *)ctxt);    \
    SDK_TRACE_DEBUG("Thread %s initializing ...",                      \
                    sdk::lib::thread::current_thread()->name());       \
}

#define SDK_THREAD_DFRD_TERM_INIT(ctxt)                                \
{                                                                      \
    sdk::lib::thread::set_current_thread((sdk::lib::thread *)ctxt);    \
    SDK_TRACE_DEBUG("Thread %s initializing ...",                      \
                    sdk::lib::thread::current_thread()->name());       \
}

class thread {
public:
    static thread *factory(const char *name, uint32_t thread_id,
                           thread_role_t thread_role, uint64_t cores_mask,
                           thread_entry_func_t entry_func,
                           uint32_t prio, int sched_policy, bool can_yield);
    static void destroy(thread *th);
    static void *dummy_entry_func(void *ctxt) { return NULL; }
    virtual sdk_ret_t start(void *ctxt);
    virtual sdk_ret_t stop(void);
    sdk_ret_t wait(void);
    sdk_ret_t wait_until_complete(void);
    const char *name(void) const { return name_; }
    uint32_t thread_id(void) const { return thread_id_; }
    uint32_t priority(void) const { return prio_; }
    uint32_t sched_policy(void) const {return sched_policy_; };
    void set_pthread_id(pthread_t pthread_id) { pthread_id_ = pthread_id; }
    pthread_t pthread_id(void) const { return pthread_id_; }
    void punch_heartbeat(void);    // punch heart-beat
    timespec_t heartbeat_ts(void) const { return hb_ts_; }
    bool can_yield(void) const { return can_yield_; }
    bool is_running(void) const { return running_; }
    void set_running(bool running) { running_ = running; }
    bool ready(void) const { return ready_; }
    void set_ready(bool ready) { ready_ = ready; }
    void set_data(void *data) { data_ = data; }
    void *data(void) const { return data_; }
    thread_role_t thread_role(void) const { return thread_role_; }
    uint64_t cores_mask(void) const { return cores_mask_; }
    uint64_t get_cpu_mask(void);

    // set the current thread instance
    static void set_current_thread(thread *curr_thread) {
        t_curr_thread_ = curr_thread;
    }

    // get the current thread instance
    static thread* current_thread(void) {
        return sdk::lib::thread::t_curr_thread_;
    }

    static void control_cores_mask_set(uint64_t mask) {
        control_cores_mask_ = mask;
        SDK_TRACE_PRINT("control_cores_mask : 0x%lx", control_cores_mask_);
    }

    static void data_cores_mask_set(uint64_t mask) {
        data_cores_mask_ = data_cores_free_ = mask;
        SDK_TRACE_PRINT("data_cores_mask : 0x%lx", data_cores_mask_);
    }
    static uint64_t get_cpu_mask(cpu_set_t cpu_set);
    static uint64_t control_cores_mask(void) { return control_cores_mask_; }
    static uint64_t data_cores_mask(void) { return data_cores_mask_; }
    static int sched_policy_by_role(thread_role_t role) {
        if (super_user_ == false) {
            return SCHED_OTHER;
        } else if (role == THREAD_ROLE_DATA) {
            return SCHED_FIFO;
        }
        return SCHED_OTHER;
    }
    static int priority_by_role(thread_role_t role) {
        int    prio, sched_policy;

        sched_policy = sched_policy_by_role(role);
        prio = sched_get_priority_max(sched_policy);
        if (sched_policy == SCHED_FIFO) {
            prio = 50;    // don't consume 100%
        }
        return prio;
    }
    // enqueue event to this thread
    bool enqueue(void *item) {
        return lfq_->enqueue(item);
    }

    // dequeue event from this thread
    void *dequeue(void) {
        return lfq_->dequeue();
    }

private:
    char                          name_[SDK_MAX_THREAD_NAME_LEN];
    uint32_t                      thread_id_;
    thread_entry_func_t           entry_func_;
    uint32_t                      prio_;
    int                           sched_policy_;
    bool                          can_yield_;
    void                          *data_;
    pthread_t                     pthread_id_;
    bool                          running_;
    bool                          ready_;
    thread_role_t                 thread_role_;
    uint64_t                      cores_mask_;
    timespec_t                    hb_ts_;
    cpu_set_t                     cpu_set_;
    lfq                           *lfq_;
    static thread_local thread    *t_curr_thread_;
    static uint64_t               control_cores_mask_;
    static uint64_t               data_cores_free_;
    static uint64_t               data_cores_mask_;
    static bool                   super_user_;

protected:
    virtual int init(const char *name, uint32_t thread_id,
        thread_role_t thread_role, uint64_t cores_mask,
        thread_entry_func_t entry_func,
        uint32_t prio, int sched_policy, bool can_yield);
    thread() {};
    ~thread();

private:
    static sdk_ret_t cores_mask_validate(thread_role_t thread_role,
                                         uint64_t mask);
};

}    // namespace lib
}    // namespace sdk

using sdk::lib::thread;

#endif    // __SDK_THREAD_HPP__

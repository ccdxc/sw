//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// thread library
//------------------------------------------------------------------------------

#ifndef __SDK_THREAD_HPP__
#define __SDK_THREAD_HPP__

#include <pthread.h>
#include "sdk/base.hpp"

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

// call this macro from the entry function of every thread
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

class thread {
public:
    static thread *factory(const char *name, uint32_t thread_id,
                           thread_role_t thread_role, uint64_t cores_mask,
                           thread_entry_func_t entry_func,
                           uint32_t prio, int sched_policy, bool can_yield);
    static void destroy(thread *th);
    static void *dummy_entry_func(void *ctxt) { return NULL; }
    sdk_ret_t start(void *ctxt);
    sdk_ret_t stop(void);
    const char *name(void) const { return name_; }
    uint32_t thread_id(void) const { return thread_id_; }
    uint32_t priority(void) const { return prio_; }
    void set_pthread_id(pthread_t pthread_id) { pthread_id_ = pthread_id; }
    pthread_t pthread_id(void) const { return pthread_id_; }
    bool is_running(void) const { return running_; }
    bool can_yield(void) const { return can_yield_; }
    void set_running(bool running) { running_ = running; }
    void set_data(void *data) { data_ = data; }

    thread_role_t thread_role (void) const { return thread_role_; }
    uint64_t      cores_mask  (void) const { return cores_mask_;  }

    void* data() {
        return data_;
    }

    // set the current thread instance
    static void set_current_thread (thread *curr_thread) {
        t_curr_thread_ = curr_thread;
    }

    // get the current thread instance
    static thread* current_thread (void) {
        return sdk::lib::thread::t_curr_thread_;
    }

    static void control_cores_mask_set (uint64_t mask) {
        control_cores_mask_ = mask;
    }

    static void data_cores_mask_set (uint64_t mask) {
        data_cores_mask_ = data_cores_free_ = mask;
    }

    static uint64_t control_cores_mask (void) { return control_cores_mask_; }
    static uint64_t data_cores_mask    (void) { return data_cores_mask_;    }

private:
    char                       name_[SDK_MAX_THREAD_NAME_LEN];
    uint32_t                   thread_id_;
    thread_entry_func_t        entry_func_;
    uint32_t                   prio_;
    int                        sched_policy_;
    bool                       can_yield_;
    void                       *data_;
    pthread_t                  pthread_id_;
    bool                       running_;
    thread_role_t              thread_role_;
    uint64_t                   cores_mask_;
    static thread_local thread *t_curr_thread_;
    static uint64_t            control_cores_mask_;
    static uint64_t            data_cores_free_;
    static uint64_t            data_cores_mask_;

private:
    thread() {};
    ~thread();
    int init(const char *name, uint32_t thread_id,
             thread_role_t thread_role, uint64_t cores_mask,
             thread_entry_func_t entry_func,
             uint32_t prio, int sched_policy, bool can_yield);

    static sdk_ret_t
    cores_mask_validate(thread_role_t thread_role, uint64_t mask);
};

}    // namespace lib
}    // namespace sdk

#endif    // __SDK_THREAD_HPP__


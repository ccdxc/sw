#ifndef __THREAD_HPP__
#define __THREAD_HPP__

#include <pthread.h>
#include <base.h>

namespace hal {
namespace utils {

//------------------------------------------------------------------------------
// thread entry function
//------------------------------------------------------------------------------
typedef void *(*hal_thread_entry_func_t)(void *ctxt);

#define HAL_MAX_THREAD_NAME_LEN        16

// call this macro from the entry function of every thread
#define HAL_THREAD_INIT()                                            \
{                                                                    \
    int __rv__, __old_type__;                                        \
                                                                     \
    __rv__ = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,      \
                                   &__old_type__);                   \
    HAL_ABORT(__rv__ == 0);                                          \
}

class thread {
public:
    static thread *factory(const char *name, uint32_t thread_id,
                           uint32_t core_id, hal_thread_entry_func_t entry_func,
                           uint32_t prio, int sched_policy, bool can_yield);
    static void *dummy_entry_func(void *ctxt) { return NULL; }
    ~thread();
    hal_ret_t start(void *ctxt);
    hal_ret_t stop(void);
    const char *name(void) const { return name_; }
    uint32_t thread_id(void) const { return thread_id_; }
    uint32_t core_id(void) const { return core_id_; }
    uint32_t priority(void) const { return prio_; }
    void set_pthread_id(pthread_t pthread_id) { pthread_id_ = pthread_id; }
    pthread_t pthread_id(void) const { return pthread_id_; }
    bool is_running(void) const { return running_; }
    void set_running(bool running) { running_ = running; }
    bool can_yield(void) const { return can_yield_; }
    void set_ctxt(void *ctxt) { ctxt_ = ctxt; }
    
private:
    char                       name_[HAL_MAX_THREAD_NAME_LEN];
    uint32_t                   thread_id_;
    uint32_t                   core_id_;
    hal_thread_entry_func_t    entry_func_;
    uint32_t                   prio_;
    int                        sched_policy_;
    bool                       can_yield_;
    void                       *ctxt_;

    pthread_t                  pthread_id_;
    bool                       running_;

private:
    thread() {};
    int init(const char *name, uint32_t thread_id, uint32_t core_id,
             hal_thread_entry_func_t entry_func,
             uint32_t prio, int sched_policy, bool can_yield);
};

}    // namespace hal
}    // namespace utils

#endif    // __THREAD_HPP__


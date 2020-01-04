//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include <unistd.h>
#include <cstring>
#include "include/sdk/base.hpp"
#include "include/sdk/mem.hpp"
#include "lib/thread/thread.hpp"
#include <sys/sysinfo.h>

namespace sdk {
namespace lib {

thread_local
thread*  thread::t_curr_thread_      = NULL;

uint64_t thread::control_cores_mask_ = 0;
uint64_t thread::data_cores_free_    = 0;
uint64_t thread::data_cores_mask_    = 0;
bool     thread::super_user_ =
             (getenv("USER") && !strcmp(getenv("USER"), "root")) ? true : false;

//------------------------------------------------------------------------------
// thread instance initialization
//------------------------------------------------------------------------------
int
thread::init(const char *name, uint32_t thread_id,
             thread_role_t thread_role, uint64_t cores_mask,
             thread_entry_func_t entry_func, uint32_t prio,
             int sched_policy, bool can_yield)
{
    if (!name || !entry_func) {
        return -1;
    }

    if (cores_mask_validate(thread_role, cores_mask) != SDK_RET_OK) {
        return -1;
    }

    strncpy(name_, name, SDK_MAX_THREAD_NAME_LEN);
    thread_id_ = thread_id;
    entry_func_ = entry_func;
    prio_ = prio;
    sched_policy_ = sched_policy;
    can_yield_ = can_yield;
    cores_mask_ = cores_mask;
    thread_role_ = thread_role;
    pthread_id_ = 0;
    running_ = false;
    memset(&hb_ts_, 0, sizeof(hb_ts_));
    lfq_ = lfq::factory(32);
    return 0;
}

sdk_ret_t
thread::cores_mask_validate(thread_role_t thread_role,
                            uint64_t mask)
{
    switch (thread_role) {
    case THREAD_ROLE_CONTROL:
        // check if the mask bits are present in control cores mask
        if (mask != 0 &&
            (mask & control_cores_mask_) == 0) {
            SDK_TRACE_ERR("Invalid control core mask 0x%lx."
                          " Expected: 0x%lx\n",
                          mask, control_cores_mask_);
            return SDK_RET_ERR;
        }
        break;

    default:
        // check if the mask bit is present in data cores mask
        if ((mask & data_cores_free_) == 0) {
            SDK_TRACE_ERR("Invalid data core mask 0x%lx."
                          " Expected, one of: 0x%lx\n",
                          mask, data_cores_free_);
            return SDK_RET_ERR;
        }

        // check if only one bit is set in mask
        if ((mask & (mask - 1)) != 0) {
            SDK_TRACE_ERR("Invalid data core mask 0x%lx."
                          " Expected, one of: 0x%lx\n",
                          mask, data_cores_free_);
            return SDK_RET_ERR;
        }

        // mark the core bit as taken
        data_cores_free_ = data_cores_free_ & ~mask;
        break;
    }

    return SDK_RET_OK;
}

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
thread *
thread::factory(const char *name, uint32_t thread_id,
                thread_role_t thread_role, uint64_t cores_mask,
                thread_entry_func_t entry_func, uint32_t prio,
                int sched_policy, bool can_yield)
{
    int       rv;
    void      *mem;
    thread    *new_thread;

    mem = SDK_CALLOC(SDK_MEM_ALLOC_LIB_THREAD, sizeof(thread));
    if (!mem) {
        return NULL;
    }
    new_thread = new (mem) thread();
    rv = new_thread->init(name, thread_id, thread_role, cores_mask,
                          entry_func, prio, sched_policy, can_yield);
    if (rv < 0) {
        new_thread->~thread();
        SDK_FREE(SDK_MEM_ALLOC_LIB_THREAD, new_thread);
        return NULL;
    }

    return new_thread;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
thread::~thread()
{
    if (running_) {
        this->stop();
    }
}

void
thread::destroy(thread *th)
{
    if (!th) {
        return;
    }
    lfq::destroy(th->lfq_);
    th->~thread();
    SDK_FREE(SDK_MEM_ALLOC_LIB_THREAD, th);
}

uint64_t
thread::get_cpu_mask (cpu_set_t cpu_set)
{
    uint64_t cpu_mask = 0x0;

    for (int i = 0; i < get_nprocs(); ++i) {
        if (CPU_ISSET(i, &cpu_set) != 0) {
            cpu_mask |= (1 << i);
        }
    }

    return cpu_mask;
}

uint64_t
thread::get_cpu_mask (void)
{
    return get_cpu_mask(cpu_set_);
}

//------------------------------------------------------------------------------
// start the thread
//------------------------------------------------------------------------------
sdk_ret_t
thread::start(void *ctxt)
{
    int                   rv;
    pthread_attr_t        attr;
    struct sched_param    sched_params;
    uint64_t              mask = cores_mask_;

    if (running_) {
        return SDK_RET_OK;
    }

    // initialize the pthread attributes
    rv = pthread_attr_init(&attr);
    if (rv != 0) {
        SDK_TRACE_ERR("pthread_attr_init failure, err : %d", rv);
        return SDK_RET_ERR;
    }

    CPU_ZERO(&cpu_set_);

    switch (thread_role_) {
    case THREAD_ROLE_CONTROL:
        if (mask == 0x0) {
            mask = thread::control_cores_mask_;
        }
        break;

    default:
        break;
    }

    // set core affinity
    if (mask != 0) {
        while (mask != 0) {
            CPU_SET(ffsl(mask) - 1, &cpu_set_);
            mask = mask & (mask - 1);
        }
        rv = pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpu_set_);
        if (rv != 0) {
            SDK_TRACE_ERR("pthread_attr_setaffinity_np failure, err : %d", rv);
            return SDK_RET_ERR;
        }
    }

    if ((sched_policy_ == SCHED_FIFO) || (sched_policy_ == SCHED_RR)) {
        // set explicit scheduling policy option
        rv = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        if (rv != 0) {
            SDK_TRACE_ERR("pthread_attr_setinheritsched failure, err : %d", rv);
            return SDK_RET_ERR;
        }

        // set the scheduling policy
        rv = pthread_attr_setschedpolicy(&attr, sched_policy_);
        if (rv != 0) {
            SDK_TRACE_ERR("pthread_attr_setschedpolicy failure, err : %d", rv);
            return SDK_RET_ERR;
        }

        // set the thread priority
        sched_params.sched_priority = prio_;
        rv = pthread_attr_setschedparam(&attr, &sched_params);
        if (rv != 0) {
            SDK_TRACE_ERR("pthread_attr_setschedparam failure, err : %d", rv);
            return SDK_RET_ERR;
        }
    }

    // create the thread now
    rv = pthread_create(&pthread_id_, &attr, entry_func_, ctxt);
    if (rv != 0) {
        int conf_cores, avail_cores;
        conf_cores = get_nprocs_conf();
        avail_cores = get_nprocs();
        SDK_TRACE_ERR("pthread_create failure, err : %d, role : %d, "
                      "cores_mask : %lu, sched_policy : %d, "
                      "sched_priority : %u, conf_cores : %d avail_cores : %d",
                      rv, thread_role_, cores_mask_, sched_policy_, prio_,
                      conf_cores, avail_cores);
        return SDK_RET_ERR;
    } else {
        int conf_cores, avail_cores;
        conf_cores = get_nprocs_conf();
        avail_cores = get_nprocs();
        SDK_TRACE_DEBUG("Instantiated thread, "
                        "name : %s, id : %u, role %u, "
                        "sched_policy : %d, priority : %u, "
                        "CPU mask: 0x%lx, cores mask : %lu, "
                        "configured cores : %d, available cores : %d",
                        name_, thread_id_, thread_role_,
                        sched_policy_, prio_,
                        get_cpu_mask(), cores_mask_,
                        conf_cores, avail_cores);
    }

    // set the thread's name, for debugging
    rv = pthread_setname_np(pthread_id_, name_);
    if (rv != 0) {
        SDK_TRACE_ERR("pthread_setname_np failure, err : %d", rv);
        return SDK_RET_ERR;
    }

    // clean up
    rv = pthread_attr_destroy(&attr);
    if (rv != 0) {
        SDK_TRACE_ERR("pthread_attr_destroy failure, err : %d", rv);
        return SDK_RET_ERR;
    }

    //  thread is ready now
    running_ = true;

    return SDK_RET_OK;
}

//------------------------------------------------------------------------------
// stop the thread
//------------------------------------------------------------------------------
sdk_ret_t
thread::stop(void)
{
    int    rv;

    if (!running_) {
        return SDK_RET_OK;
    }

    rv = pthread_cancel(pthread_id_);
    if (rv != 0) {
        SDK_TRACE_ERR("pthread cancel failed on thread %s", name_);
    }

    running_ = false;
    return SDK_RET_OK;
}

//------------------------------------------------------------------------------
// wait for the thread to complete
//------------------------------------------------------------------------------
sdk_ret_t
thread::wait_until_complete(void)
{
    int    rv;
    void   *res;

    rv = pthread_join(pthread_id_, &res);
    if (rv != 0) {
        SDK_TRACE_ERR("pthread cancel failed on thread %s", name_);
    }
    SDK_TRACE_DEBUG("pthread exit code for thread %s is %s", name_,
                    res == PTHREAD_CANCELED ? "cancelled" : "exit");
    return SDK_RET_OK;
}

//------------------------------------------------------------------------------
// wait for the thread to complete. Thread needs to be stopped first.
//------------------------------------------------------------------------------
sdk_ret_t
thread::wait(void)
{
    if (running_) {
        SDK_TRACE_ERR("pthread cancel not done on thread %s", name_);
        return SDK_RET_ERR;
    }

    return wait_until_complete();
}

//------------------------------------------------------------------------------
// punch heart beat
//------------------------------------------------------------------------------
void
thread::punch_heartbeat(void)
{
    clock_gettime(CLOCK_MONOTONIC, &hb_ts_);
}

}    // namespace lib
}    // namespace sdk

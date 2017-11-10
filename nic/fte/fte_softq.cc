#include <atomic>
#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/thread.hpp"
#include "fte_softq.hpp"

namespace fte {

#define MAX_FTE_THREADS (hal::HAL_THREAD_ID_FTE_MAX - hal::HAL_THREAD_ID_FTE_MIN + 1)

//------------------------------------------------------------------------------
// Per FTE SoftQs
//------------------------------------------------------------------------------
static mpscq_t *g_softqs[MAX_FTE_THREADS];

inline bool is_fte_thread()
{
    uint32_t tid = hal::utils::thread::current_thread()->thread_id();
    return tid >= hal::HAL_THREAD_ID_FTE_MIN && tid <= hal::HAL_THREAD_ID_FTE_MAX;
}

//------------------------------------------------------------------------------
// Asynchronouly executes the fn in the specified fte thread,
// If the softq is full, it blocks until a slot is empty.
//-----------------------------------------------------------------------------
hal_ret_t softq_enqueue(uint8_t fte_id, softq_fn_t fn, void *data)
{
    hal::utils::thread *curr_thread = hal::utils::thread::current_thread();

    HAL_ASSERT(is_fte_thread() == false);

    HAL_ASSERT_RETURN(fte_id < MAX_FTE_THREADS, HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN(fn, HAL_RET_INVALID_ARG);

    mpscq_t *softq = g_softqs[fte_id];
    if (softq == NULL) {
        HAL_TRACE_ERR("fte: softq is not initialized for fte.{}", fte_id);
        return HAL_RET_ERR;
    }

    HAL_TRACE_DEBUG("fte: softq enqueue fte.{} fn={:p} data={:p}", fte_id, (void*)fn, data);
    // try indefinatly until queued successfully
    while(softq->enqueue((void *)fn, data) == false) {
        if (curr_thread->can_yield()) {
            pthread_yield();
        }
    }

    return HAL_RET_OK;
}


//------------------------------------------------------------------------------
// Initialize softq for the current fte thread and store it in the gloal list
//------------------------------------------------------------------------------
hal_ret_t softq_register(mpscq_t *q)
{
    HAL_ASSERT(is_fte_thread());

    uint8_t fte_id = hal::utils::thread::current_thread()->thread_id() - hal::HAL_THREAD_ID_FTE_MIN;

    HAL_ASSERT(g_softqs[fte_id] == NULL);

    g_softqs[fte_id] = q;

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Queues a handler in softq ,
// Returns false if queue is full
//-----------------------------------------------------------------------------
bool mpscq_t::enqueue(void *op, void *data)
{
    uint16_t pi = pi_.load();

    do {
        if(slots_[pi].full) {
            return false;
        }
    } while(!pi_.compare_exchange_weak(pi, (pi+1)%nslots_,
                                       std::memory_order_release,
                                       std::memory_order_relaxed));

    slots_[pi].op = op;
    slots_[pi].data = data;
    slots_[pi].full.store(true);
    return true;
}

//------------------------------------------------------------------------------
// Dequeus an handler from the queue
// Returns false if queue is empty
//-----------------------------------------------------------------------------
bool mpscq_t::dequeue(void **op, void **data)
{
    if (!slots_[ci_].full) {
         return false;
    }

    if (op) {
        *op = slots_[ci_].op;
    }

    if (data) {
        *data = slots_[ci_].data;
    }

    slots_[ci_].full.store(false);
    ci_ = (ci_+1) % nslots_;
    return true;
}

}




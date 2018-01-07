#include <atomic>
#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "sdk/thread.hpp"
#include "fte_softq.hpp"

namespace fte {

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




#include <atomic>
#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "lib/thread/thread.hpp"
#include "fte_softq.hpp"

namespace fte {

//------------------------------------------------------------------------------
// Queues a handler in softq ,
// Returns false if queue is full
//-----------------------------------------------------------------------------
bool mpscq_t::enqueue(void *op, void *data)
{
    uint16_t pi = SDK_ATOMIC_LOAD_BOOL(&pi_);

    do {
        if(slots_[pi].full) {
            return false;
        }
    } while(!SDK_ATOMIC_COMPARE_EXCHANGE_WEAK(&pi_, &pi, 
                                              (pi+1)%nslots_));

    slots_[pi].op = op;
    slots_[pi].data = data;
    SDK_ATOMIC_STORE_BOOL(&slots_[pi].full, true);

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

    SDK_ATOMIC_STORE_BOOL(&slots_[ci_].full, false);

    ci_ = (ci_+1) % nslots_;
    return true;
}

}


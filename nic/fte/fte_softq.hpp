#pragma once

#include "nic/include/base.h"

namespace fte {

class mpscq_t;

typedef void (*softq_fn_t)(void *data);

// Should be called from non FTE thread 
hal_ret_t softq_enqueue(uint8_t fte_id, softq_fn_t fn, void *data);

// Should be called from FTE thread 
hal_ret_t softq_register(mpscq_t *q);

// ------------------------------------------------------------------------------
// Multiple producer single consumer queue 
// ------------------------------------------------------------------------------
class  mpscq_t {
public:
    // non-blocking enqueue - returns false if queue is full
    bool enqueue(void *op, void *data);

    //  non-blocking dequeue - Returns false if queue is empty
    bool dequeue(void **op, void **data);

    static mpscq_t *alloc(uint16_t nslots) { return new mpscq_t(nslots); }
private:
    mpscq_t(uint16_t nslots): nslots_(nslots), pi_(0), ci_(0), slots_(new slot_t[nslots]) {}
    uint16_t nslots_;           // max no of slots
    std::atomic<uint16_t> pi_;  // producer index (shared by producers)
    uint16_t ci_;               // consumer index (local to consumer)
    struct slot_t {
        std::atomic<bool>    full;    // TRUE if slot is full
                                      // (shared between producers and consumer)
        void                *op;      // Queued op
        void                *data;    // Queued data
        slot_t(): full(false), op(NULL), data(NULL){}
    } *slots_;
};

}

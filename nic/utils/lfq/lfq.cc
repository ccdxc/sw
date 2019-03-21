// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <boost/lockfree/queue.hpp>
#include "lfq.hpp"

namespace hal {
namespace utils {

class lfq::lfq_impl {
public:
    static lfq_impl *factory(uint32_t size=0) {
        void        *mem;
        lfq_impl    *new_lfq_impl;

        mem = HAL_CALLOC(HAL_MEM_ALLOC_INFRA, sizeof(lfq_impl));
        if (mem == NULL) {
            return NULL;
        }
        if (size) {
            new_lfq_impl = new (mem) lfq_impl(size);
        } else {
            new_lfq_impl = new (mem) lfq_impl();
        }
        if (!new_lfq_impl->queue_.is_lock_free()) {
            new_lfq_impl->~lfq_impl();
            HAL_FREE(HAL_MEM_ALLOC_INFRA, mem);
            return NULL;
        }
        return new_lfq_impl;
    }
    static void destroy(lfq_impl *q_impl) {
        q_impl->~lfq_impl();
        HAL_FREE(HAL_MEM_ALLOC_INFRA, q_impl);
    }
    bool enqueue(void *item) { return queue_.push(item); }
    void *dequeue(void) {
        void    *item;

        if (queue_.pop(item)) {
            return item;
        }
        return NULL;
    }

private:
    boost::lockfree::queue<void *>    queue_;

private:
    lfq_impl(uint32_t size):queue_(size) {}
    lfq_impl():queue_() {}
    ~lfq_impl() {}
};

lfq::lfq() {
    lfq_impl_ = NULL;
}

lfq *
lfq::factory(uint32_t size) {
     void        *mem;
     lfq         *new_lfq;
     lfq_impl    *new_lfq_impl;

     mem = HAL_CALLOC(HAL_MEM_ALLOC_INFRA, sizeof(lfq));
     if (mem == NULL) {
         return NULL;
     }
     new_lfq = new (mem) lfq();

     new_lfq_impl = lfq_impl::factory(size);
     if (new_lfq_impl == NULL) {
         goto end;
     }
    new_lfq->lfq_impl_ = new_lfq_impl;
    return new_lfq;

end:

    if (new_lfq) {
        new_lfq->~lfq();
        HAL_FREE(HAL_MEM_ALLOC_INFRA, new_lfq);
    }
    return NULL;
}

void
lfq::destroy(lfq *q) {
    lfq_impl::destroy(q->lfq_impl_);
    q->lfq_impl_ = NULL;
    q->~lfq();
    HAL_FREE(HAL_MEM_ALLOC_INFRA, q);
}

}    // namespace utils
}    // namespace hal

//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// lock free multi-producer, multi-consumer queue
//------------------------------------------------------------------------------

#ifndef __LFQ_HPP__
#define __LFQ_HPP__

#include "nic/include/base.h"
#include "nic/include/hal_mem.hpp"

namespace hal {
namespace utils {

class lfq {
public:
    static lfq *factory(uint32_t size=0);
    static void destroy(lfq *q);
    bool enqueue(void *item);
    void *dequeue(void);

private:
    class       lfq_impl;
    lfq_impl    *lfq_impl_;

private:
    lfq();
    ~lfq() {}
};

}    // namespace utils
}    // namespace hal

#endif    // __LFQ_HPP__


#ifndef	ACC_RING_HPP_
#define	ACC_RING_HPP_

#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdio.h>

#include "dp_mem.hpp"

using namespace dp_mem;

namespace tests {

/*
 * acc_ring_t provides usage flexibility as follows:
 * - HW ring configuration performed by this DOL module or elsewhere (such as HAL)
 * - Ring entry submission via sequencer or directly to HW producer register
 * - Sequencer batch support
 */
typedef enum {
    ACC_RING_PUSH_INVALID,
    ACC_RING_PUSH_SEQUENCER,
    ACC_RING_PUSH_SEQUENCER_BATCH,
    ACC_RING_PUSH_HW_DIRECT,
    ACC_RING_PUSH_HW_DIRECT_BATCH,
    ACC_RING_PUSH_HW_INDIRECT_BATCH,
} acc_ring_push_t;

class acc_ring_t
{
public:
    acc_ring_t(uint64_t cfg_ring_pd_idx,
               uint32_t ring_size,
               uint32_t desc_size,
               uint64_t ring_base_mem_pa = 0,
               uint32_t pi_size = sizeof(uint32_t));
    void push(const void *src_desc,
              acc_ring_push_t push_type,
              uint32_t seq_qid);
    void reentrant_tuple_set(acc_ring_push_t push_type,
                             uint32_t seq_qid);
    void post_push(uint16_t push_amount = 0);

    uint32_t ring_size_get(void)
    {
        return ring_size;
    }

    uint32_t ring_desc_size_get(void)
    {
        return desc_size;
    }

    uint32_t ring_pi_size_get(void)
    {
        return pi_size;
    }

    uint64_t ring_base_mem_pa_get(void)
    {
        return ring_base_mem_pa;
    }

    uint64_t cfg_ring_pd_idx_get(void)
    {
        return cfg_ring_pd_idx;
    }

    uint64_t shadow_pd_idx_pa_get(void)
    {
        return shadow_pd_idx_mem->pa();
    }

    uint16_t seq_pd_idx_get(void)
    {
        return curr_seq_pd_idx;
    }

    uint16_t ring_pd_idx_get(void)
    {
        return curr_pd_idx;
    }

private:
    uint64_t        cfg_ring_pd_idx;
    dp_mem_t        *shadow_pd_idx_mem;
    uint8_t         *ring_base_mem;
    uint64_t        ring_base_mem_pa;

    uint32_t        ring_size;
    uint32_t        desc_size;
    uint32_t        pi_size;
    uint32_t        curr_seq_qid;
    uint16_t        curr_seq_pd_idx;
    uint16_t        curr_pd_idx;
    uint16_t        prev_pd_idx;
    acc_ring_push_t curr_push_type;
};

}  // namespace tests

#endif /// ACC_RING_HPP_

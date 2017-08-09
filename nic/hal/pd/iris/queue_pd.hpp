#ifndef __HAL_QUEUE_PD_HPP__
#define __HAL_QUEUE_PD_HPP__

#include <base.h>
#include <qos.hpp>
#include <pd.hpp>
#include <pd_api.hpp>
#include <capri_tm_rw.hpp>


namespace hal {
namespace pd {

#define HAL_INVALID_HW_QUEUE_ID 0xffffffff

struct pd_queue_s {
    uint32_t    hw_queue_id;
    uint32_t    dummy;

    // pi ptr
    void        *pi_queue;
} __PACK__;


pd_queue_t *queue_pd_alloc ();
pd_queue_t *queue_pd_init (pd_queue_t *queue);
pd_queue_t *queue_pd_alloc_init ();
hal_ret_t queue_pd_free (pd_queue_t *queue);
hal_ret_t queue_pd_alloc_res(pd_queue_t *pd_queue);
void queue_pd_dealloc_res(pd_queue_t *pd_queue);
hal_ret_t queue_pd_program_hw(pd_queue_args_t *args);
void link_pi_pd(pd_queue_t *pd_queue, queue_t *pi_queue);
void unlink_pi_pd(pd_queue_t *pd_queue, queue_t *pi_queue);

static inline uint32_t
queue_count_by_node_type(uint32_t port, queue_node_type_e node_type)
{
    switch(node_type) {
        case L0_NODE:
            return HAL_TM_COUNT_L0_NODES;
        case L1_NODE:
            return HAL_TM_COUNT_L1_NODES;
        case L2_NODE:
            return HAL_TM_COUNT_L2_NODES;
    }
    return 0;
}

}   // namespace pd
}   // namespace hal

#endif    // __HAL_QUEUE_PD_HPP__

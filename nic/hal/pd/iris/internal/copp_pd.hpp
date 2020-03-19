#ifndef __HAL_COPP_PD_HPP__
#define __HAL_COPP_PD_HPP__

#include "nic/include/base.hpp"
#include "nic/hal/plugins/cfg/aclqos/qos.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"


namespace hal {
namespace pd {

struct pd_copp_s {
    uint32_t hw_policer_id;

    // pi ptr
    copp_t   *pi_copp;
} __PACK__;


// allocate Copp Instance
static inline pd_copp_t *
copp_pd_alloc (void)
{
    pd_copp_t    *copp;

    copp = (pd_copp_t *)g_hal_state_pd->copp_pd_slab()->alloc();
    if (copp == NULL) {
        return NULL;
    }

    return copp;
}

// initialize Copp PD instance
static inline pd_copp_t *
copp_pd_init (pd_copp_t *copp)
{
    // Nothing to do currently
    if (!copp) {
        return NULL;
    }

    // Set here if you want to initialize any fields
    copp->hw_policer_id = INVALID_INDEXER_INDEX;

    return copp;
}

// allocate and Initialize Copp PD Instance
static inline pd_copp_t *
copp_pd_alloc_init(void)
{
    return copp_pd_init(copp_pd_alloc());
}


// freeing Copp PD memory
static inline hal_ret_t
copp_pd_mem_free (pd_copp_t *pd_copp)
{
    if (pd_copp) {
        hal::pd::delay_delete_to_slab(HAL_SLAB_COPP_PD, pd_copp);
    }
    return HAL_RET_OK;
}

// freeing Copp PD
static inline hal_ret_t
copp_pd_free (pd_copp_t *pd_copp)
{
    return copp_pd_mem_free(pd_copp);
}

}   // namespace pd
}   // namespace hal

#endif    // __HAL_COPP_PD_HPP__

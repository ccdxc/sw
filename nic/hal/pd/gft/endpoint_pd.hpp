// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_ENDPOINT_PD_HPP__
#define __HAL_ENDPOINT_PD_HPP__

#include "nic/include/base.h"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/gft/gft_state.hpp"

namespace hal {
namespace pd {

struct pd_ep_s {
    uint32_t    tx_vport_idx;
    // Only for DOLs to support Uplink -> Ingress -> Uplink path
    uint32_t    rx_vport_idx;

    // pi ptr
    void        *pi_ep;
} __PACK__;

// allocate EP Instance
static inline pd_ep_t *
ep_pd_alloc (void)
{
    pd_ep_t    *ep;

    ep = (pd_ep_t *)g_hal_state_pd->ep_pd_slab()->alloc();
    if (ep == NULL) {
        return NULL;
    }

    return ep;
}

// initialize EP PD instance
static inline pd_ep_t *
ep_pd_init (pd_ep_t *ep)
{
    // Nothing to do currently
    if (!ep) {
        return NULL;
    }

    // Set here if you want to initialize any fields

    return ep;
}

// allocate and Initialize EP PD Instance
static inline pd_ep_t *
ep_pd_alloc_init(void)
{
    return ep_pd_init(ep_pd_alloc());
}

// freeing EP PD
static inline hal_ret_t
ep_pd_free (pd_ep_t *ep)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_EP_PD, ep);
    return HAL_RET_OK;
}


// freeing EP PD memory
static inline hal_ret_t
ep_pd_mem_free (pd_ep_t *ep)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_EP_PD, ep);
    return HAL_RET_OK;
}


}   // namespace pd
}   // namespace hal
#endif    // __HAL_ENDPOINT_PD_HPP__


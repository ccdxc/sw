#include <netinet/ether.h>
#include "nic/include/hal_lock.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/l4lb_pd.hpp"
// #include "nic/include/endpoint_api.hpp"
// #include "nic/include/interface_api.hpp"
// #include "nic/hal/pd/iris/endpoint_pd.hpp"
// #include "nic/hal/pd/iris/lif_pd.hpp"
// #include "nic/hal/src/l2segment.hpp"
// #include "nic/hal/src/network.hpp"
// #include "nic/p4/nw/include/defines.h"
#include "nic/hal/pd/iris/if_pd_utils.hpp"
// #include "nic/hal/src/utils.hpp"

using namespace hal;

namespace hal {
namespace pd {

// ----------------------------------------------------------------------------
// l4lb Create
// ----------------------------------------------------------------------------
hal_ret_t 
pd_l4lb_create(pd_l4lb_args_t *args)
{
    hal_ret_t               ret = HAL_RET_OK;; 
    pd_l4lb_t               *pd_l4lb;
    mac_addr_t              *mac;

    mac = &args->l4lb->serv_mac_addr;

    HAL_TRACE_DEBUG("PD-L4LB::{}: Creating pd state for L4LB: serv_mac:{}", 
                    __FUNCTION__, 
                    ether_ntoa((struct ether_addr*)*mac));


    // Create l4lb PD
    pd_l4lb = l4lb_pd_alloc_init();
    if (pd_l4lb == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    link_pi_pd(pd_l4lb, args->l4lb);

    // Allocate Resources
    ret = l4lb_pd_alloc_res(pd_l4lb);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("PD-L4LB::{}: Unable to alloc. resources for L4LB: serv_mac:{}", 
                __FUNCTION__, 
                ether_ntoa((struct ether_addr*)*mac));
        goto end;
    }

    // Program HW
    ret = l4lb_pd_program_hw(pd_l4lb);

end:
    if (ret != HAL_RET_OK) {
        unlink_pi_pd(pd_l4lb, args->l4lb);
        l4lb_pd_free(pd_l4lb);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Allocate and Initialize L4LB PD Instance
// ----------------------------------------------------------------------------
pd_l4lb_t *
l4lb_pd_alloc_init(void)
{
    return l4lb_pd_init(l4lb_pd_alloc());
}

// ----------------------------------------------------------------------------
// Allocate L4LB Instance
// ----------------------------------------------------------------------------
pd_l4lb_t *
l4lb_pd_alloc (void)
{
    pd_l4lb_t    *l4lb;

    l4lb = (pd_l4lb_t *)g_hal_state_pd->l4lb_pd_slab()->alloc();
    if (l4lb == NULL) {
        return NULL;
    }

    return l4lb;
}

// ----------------------------------------------------------------------------
// Initialize L4LB PD instance
// ----------------------------------------------------------------------------
pd_l4lb_t *
l4lb_pd_init (pd_l4lb_t *l4lb)
{
    // Nothing to do currently
    if (!l4lb) {
        return NULL;
    }

    // Set here if you want to initialize any fields

    return l4lb;
}

// ----------------------------------------------------------------------------
// Allocate resources for PD L4LB
// ----------------------------------------------------------------------------
hal_ret_t 
l4lb_pd_alloc_res(pd_l4lb_t *pd_l4lb)
{
    hal_ret_t            ret = HAL_RET_OK;

#if 0
    indexer::status      rs = indexer::SUCCESS;
    // Allocate lif hwid
    rs = g_hal_state_pd->lif_hwid_idxr()->alloc((uint32_t *)&pd_l4lb->hw_lif_id);
    if (rs != indexer::SUCCESS) {
        return HAL_RET_NO_RESOURCE;
    }
#endif

    return ret;
}

// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
hal_ret_t
l4lb_pd_program_hw(pd_l4lb_t *pd_l4lb)
{
    hal_ret_t            ret = HAL_RET_OK;

    // Program RW Table
    // ret = l4lb_pd_pgm_rw_tbl(pd_l4lb);

    return ret;
}

// ----------------------------------------------------------------------------
// Eventually clean this up to have a hash table instead of blindly installing
// all entries.
// ----------------------------------------------------------------------------
hal_ret_t
l4lb_pd_pgm_rw_tbl(pd_l4lb_t *pd_l4lb)
{
    hal_ret_t            ret = HAL_RET_OK;
    mac_addr_t           mac_sa, mac_da;
    rewrite_actiondata   data;
    DirectMap            *rw_tbl = NULL;
    mac_addr_t           *mac;
    l4lb_service_entry_t *pi_l4lb = (l4lb_service_entry_t *)pd_l4lb->pi_l4lb;

    memset(mac_sa, 0, sizeof(mac_sa));
    memset(mac_da, 0, sizeof(mac_da));
    memset(&data, 0, sizeof(data));
    
    mac = &pi_l4lb->serv_mac_addr;

    rw_tbl = g_hal_state_pd->dm_table(P4TBL_ID_REWRITE);
    HAL_ASSERT_RETURN((rw_tbl != NULL), HAL_RET_ERR);

    // mac_sa = ... Get it from l2seg rmac
    // mac_da = ... Get it from EP's NH
    // Start by skipping REWRITE_NOP_ID
    for (int i = 1; i < REWRITE_MAX_ID; i++) {
        switch(i) {
            case REWRITE_REWRITE_ID:
            case REWRITE_IPV4_NAT_SRC_REWRITE_ID:
            case REWRITE_IPV4_NAT_DST_REWRITE_ID:
            case REWRITE_IPV4_NAT_SRC_UDP_REWRITE_ID:
            case REWRITE_IPV4_NAT_SRC_TCP_REWRITE_ID:
            case REWRITE_IPV4_NAT_DST_UDP_REWRITE_ID:
            case REWRITE_IPV4_NAT_DST_TCP_REWRITE_ID:
            case REWRITE_IPV4_TWICE_NAT_REWRITE_ID:
            case REWRITE_IPV4_TWICE_NAT_UDP_REWRITE_ID:
            case REWRITE_IPV4_TWICE_NAT_TCP_REWRITE_ID:
            case REWRITE_IPV6_NAT_SRC_REWRITE_ID:
            case REWRITE_IPV6_NAT_DST_REWRITE_ID:
            case REWRITE_IPV6_NAT_SRC_UDP_REWRITE_ID:
            case REWRITE_IPV6_NAT_SRC_TCP_REWRITE_ID:
            case REWRITE_IPV6_NAT_DST_UDP_REWRITE_ID:
            case REWRITE_IPV6_NAT_DST_TCP_REWRITE_ID:
            case REWRITE_IPV6_TWICE_NAT_REWRITE_ID:
            case REWRITE_IPV6_TWICE_NAT_UDP_REWRITE_ID:
            case REWRITE_IPV6_TWICE_NAT_TCP_REWRITE_ID:
                memcpy(data.rewrite_action_u.rewrite_rewrite.mac_sa, *mac, 6);
                memrev(data.rewrite_action_u.rewrite_rewrite.mac_sa, 6);
                break;
            default:
                HAL_ASSERT(0);
        }
        data.actionid = i;
        ret = rw_tbl->insert(&data, &(pd_l4lb->rw_tbl_idx[i]));
        if (ret != HAL_RET_OK) {
            HAL_TRACE_DEBUG("PD-EP::{}: Unable to program for L4LB: serv_mac:{}", 
                    __FUNCTION__, 
                    ether_ntoa((struct ether_addr*)*mac));
        } else {
            HAL_TRACE_DEBUG("PD-EP::{}: Programmed for L4LB: serv_mac:{} at {}", 
                    __FUNCTION__, 
                    ether_ntoa((struct ether_addr*)*mac),
                    pd_l4lb->rw_tbl_idx[i]);
        }
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Freeing L4LB PD
// ----------------------------------------------------------------------------
hal_ret_t
l4lb_pd_free (pd_l4lb_t *l4lb)
{
    g_hal_state_pd->l4lb_pd_slab()->free(l4lb);
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
link_pi_pd(pd_l4lb_t *pd_l4lb, l4lb_service_entry_t *pi_l4lb)
{
    pd_l4lb->pi_l4lb = pi_l4lb;
    pi_l4lb->pd = pd_l4lb;
}

// ----------------------------------------------------------------------------
// Un-Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
unlink_pi_pd(pd_l4lb_t *pd_l4lb, l4lb_service_entry_t *pi_l4lb)
{
    pd_l4lb->pi_l4lb = NULL;
    pi_l4lb->pd = NULL;
}

// ----------------------------------------------------------------------------
// Input: PI L4LB and Rewrite Action
// Retuns: RW idx in flow table
// ----------------------------------------------------------------------------
uint32_t 
l4lb_pd_get_rw_tbl_idx_from_pi_l4lb(l4lb_service_entry_t *pi_l4lb, rewrite_actions_en rw_act)
{
    pd_l4lb_t *pd_l4lb = NULL;

    pd_l4lb = (pd_l4lb_t *)pi_l4lb->pd;

    return l4lb_pd_get_rw_tbl_idx(pd_l4lb, rw_act);
}

// ----------------------------------------------------------------------------
// Input: Dest PD EP and Rewrite Action
// Retuns: RW idx in flow table
// ----------------------------------------------------------------------------
uint32_t
l4lb_pd_get_rw_tbl_idx(pd_l4lb_t *pd_l4lb, rewrite_actions_en rw_act)
{
    HAL_ASSERT(rw_act < REWRITE_MAX_ID);

    HAL_TRACE_DEBUG("rw_act:{}, rw_idx:{}", rw_act, 
            pd_l4lb->rw_tbl_idx[rw_act]);
    return pd_l4lb->rw_tbl_idx[rw_act];
}

}    // namespace pd
}    // namespace hal

// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <netinet/ether.h>
#include "nic/hal/pd/gft/endpoint_pd.hpp"
#include "nic/hal/pd/gft/pd_utils.hpp"
#include "nic/include/endpoint_api.hpp"
#include "nic/hal/pd/globalpd/gpd_utils.hpp"

using namespace hal;

namespace hal {
namespace pd {

// ----------------------------------------------------------------------------
// Prototypes
// ----------------------------------------------------------------------------
static hal_ret_t ep_pd_pgm_tx_vport (pd_ep_t *pd_ep, table_oper_t oper);
static hal_ret_t ep_pd_alloc_res(pd_ep_t *up_ep);
static hal_ret_t ep_pd_dealloc_res(pd_ep_t *up_ep);
static hal_ret_t ep_pd_cleanup(pd_ep_t *ep_pd);
static hal_ret_t ep_pd_program_hw(pd_ep_t *up_ep);
static void ep_link_pi_pd(pd_ep_t *pd_ep, ep_t *pi_ep);
static void ep_delink_pi_pd(pd_ep_t *pd_ep, ep_t *pi_up_ep);

// ----------------------------------------------------------------------------
// EP Create
// ----------------------------------------------------------------------------
hal_ret_t 
pd_ep_create(pd_ep_create_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;; 
    pd_ep_t             *pd_ep;
    mac_addr_t           *mac;

    mac = ep_get_mac_addr(args->ep);

    HAL_TRACE_DEBUG("{}: creating pd state for ep: {}", 
                    __FUNCTION__, ep_l2_key_to_str(args->ep));
                    
    // Create ep PD
    pd_ep = ep_pd_alloc_init();
    if (pd_ep == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    ep_link_pi_pd(pd_ep, args->ep);

    // Allocate Resources
    ret = ep_pd_alloc_res(pd_ep);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("PD-EP::{}: Unable to alloc. resources for EP: {}:{}", 
                      __FUNCTION__, ep_get_l2segid(args->ep), 
                ether_ntoa((struct ether_addr*)*mac));
        goto end;
    }

    // Program HW
    ret = ep_pd_program_hw(pd_ep);

end:
    if (ret != HAL_RET_OK) {
        // unlink_pi_pd(pd_ep, args->ep);
        // ep_pd_free(pd_ep);
        ep_pd_cleanup(pd_ep);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// EP Update 
// ----------------------------------------------------------------------------
hal_ret_t 
pd_ep_update (pd_ep_update_args_t *pd_ep_upd_args)
{
    hal_ret_t           ret = HAL_RET_OK;

    HAL_TRACE_DEBUG(":{}: updating pd state for ep:{}", 
                    __FUNCTION__,
                    ep_l2_key_to_str(pd_ep_upd_args->ep));

    return ret;
}

//-----------------------------------------------------------------------------
// PD Endpoint Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_ep_delete (pd_ep_delete_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_ep_t    *ep_pd;

    HAL_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->ep != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->ep->pd != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("{}:deleting pd state for ep {}",
                    __FUNCTION__, ep_l2_key_to_str(args->ep));
    ep_pd = (pd_ep_t *)args->ep->pd;

    // deprogram hw

    // free up the resource and memory
    ret = ep_pd_cleanup(ep_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed pd ep delete",
                      __FUNCTION__);
        goto end;
    }

end:
    return ret;
}

//-----------------------------------------------------------------------------
// PD Endpoint Cleanup
//  - Release all resources
//  - Delink PI <-> PD
//  - Free PD Endpoint
//  Note:
//      - Just free up whatever PD has. 
//      - Dont use this inplace of delete. Delete may result in giving callbacks
//        to others.
//-----------------------------------------------------------------------------
hal_ret_t
ep_pd_cleanup(pd_ep_t *ep_pd)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!ep_pd) {
        // Nothing to do
        goto end;
    }

    // Releasing resources
    ret = ep_pd_dealloc_res(ep_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: unable to dealloc res for ep: {}", 
                      __FUNCTION__, 
                      (ep_l2_key_to_str((ep_t *)(ep_pd->pi_ep))));
        goto end;
    }

    // Delinking PI<->PD
    ep_delink_pi_pd(ep_pd, (ep_t *)ep_pd->pi_ep);

    // Freeing PD
    ep_pd_free(ep_pd);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// Allocate resources for PD EP
// ----------------------------------------------------------------------------
static hal_ret_t 
ep_pd_alloc_res(pd_ep_t *pd_ep)
{
    hal_ret_t            ret = HAL_RET_OK;

    return ret;
}

// ----------------------------------------------------------------------------
// De-Allocate resources for PD EP
// ----------------------------------------------------------------------------
static hal_ret_t 
ep_pd_dealloc_res(pd_ep_t *pd_ep)
{
    hal_ret_t            ret = HAL_RET_OK;

    return ret;
}

// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
static hal_ret_t
ep_pd_program_hw(pd_ep_t *pd_ep)
{
    hal_ret_t            ret = HAL_RET_OK;

    // Program Tx Vport table
    ret = ep_pd_pgm_tx_vport(pd_ep, TABLE_OPER_INSERT);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to program tx_vport_table. ret:{}", ret);
        goto end;
    }

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Program Tx Vport
// ----------------------------------------------------------------------------
static hal_ret_t
ep_pd_pgm_tx_vport (pd_ep_t *pd_ep, table_oper_t oper)
{
    hal_ret_t               ret = HAL_RET_OK;
    sdk_ret_t               sdk_ret;
    tx_vport_swkey_t        key;
    tx_vport_swkey_mask_t   mask;
    tx_vport_actiondata     data;
    mac_addr_t              *mac = NULL;
    tcam                    *tx_vport_tbl = NULL;
    ep_t                    *pi_ep = (ep_t *)pd_ep->pi_ep;
    if_t                    *pi_if = NULL;
    uint32_t                hw_lif_id = 0;

    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    pi_if = find_if_by_handle(pi_ep->if_handle);

    tx_vport_tbl = g_hal_state_pd->tcam_table(P4TBL_ID_TX_VPORT);
    HAL_ASSERT_RETURN((tx_vport_tbl != NULL), HAL_RET_ERR);


    // key
    mac = ep_get_mac_addr(pi_ep);
    memcpy(key.flow_action_metadata_tx_ethernet_dst, *mac, 
           ETHER_ADDR_LEN);
    memrev(key.flow_action_metadata_tx_ethernet_dst, ETHER_ADDR_LEN);

    // mask
    memset(mask.flow_action_metadata_tx_ethernet_dst_mask, 0xFF, 6);

    if_get_hw_lif_id(pi_if, &hw_lif_id);
    data.tx_vport_action_u.tx_vport_tx_vport.port = hw_lif_id;
    // TODO: Take it from config
    // data.rx_vport_action_u.rx_vport_rx_vport.rdma_enabled = 1;

    if (oper == TABLE_OPER_INSERT) {
        sdk_ret = tx_vport_tbl->insert(&key, &mask, &data,
                                       &pd_ep->tx_vport_idx);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("unable to program tx_vport tbl. ret: {}",
                          ret);
            goto end;
        } else {
            HAL_TRACE_DEBUG("programmed tx_vport tbl at: {}",
                            pd_ep->tx_vport_idx);
        }
    } else {
        sdk_ret = tx_vport_tbl->update(pd_ep->tx_vport_idx, 
                                       &data);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("unable to program tx_vport tbl. ret: {}",
                          ret);
            goto end;
        } else {
            HAL_TRACE_DEBUG("programmed tx_vport tbl at: {}",
                            pd_ep->tx_vport_idx);
        }
    }

end:

    return ret;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
ep_link_pi_pd(pd_ep_t *pd_ep, ep_t *pi_ep)
{
    pd_ep->pi_ep = pi_ep;
    ep_set_pd_ep(pi_ep, pd_ep);
}

// ----------------------------------------------------------------------------
// De-Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
ep_delink_pi_pd(pd_ep_t *pd_ep, ep_t *pi_ep)
{
    pd_ep->pi_ep = NULL;
    ep_set_pd_ep(pi_ep, NULL);
}

// ----------------------------------------------------------------------------
// Makes a clone
// ----------------------------------------------------------------------------
hal_ret_t
pd_ep_make_clone(pd_ep_make_clone_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_ep_t             *pd_ep_clone = NULL;
    ep_t *ep = args->ep;
    ep_t *clone = args->clone;

    pd_ep_clone = ep_pd_alloc_init();
    if (pd_ep_clone == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    memcpy(pd_ep_clone, ep->pd, sizeof(pd_ep_t));

    ep_link_pi_pd(pd_ep_clone, clone);

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Frees PD memory without indexer free.
// ----------------------------------------------------------------------------
hal_ret_t
pd_ep_mem_free(pd_ep_mem_free_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_ep_t        *ep_pd;

    ep_pd = (pd_ep_t *)args->ep->pd;
    ep_pd_mem_free(ep_pd);

    return ret;
}

}    // namespace pd
}    // namespace hal

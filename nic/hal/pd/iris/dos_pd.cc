#include "nic/include/base.h"
#include "nic/include/hal_lock.hpp"
#include "nic/gen/iris/include/p4pd.h"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/dos_pd.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/include/dos_api.hpp"

namespace hal {
namespace pd {

//-----------------------------------------------------------------------------
// PD DoS Create
//-----------------------------------------------------------------------------
hal_ret_t
pd_dos_policy_create (pd_dos_policy_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;; 
    pd_dos_policy_t      *pd_dosp;

    HAL_TRACE_DEBUG("pd-dos:{}: creating pd state ",
                    __FUNCTION__);

    // Create dos PD
    pd_dosp = dos_pd_alloc_init();
    if (pd_dosp == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    dos_link_pi_pd(pd_dosp, args->dos_policy);

    // Allocate Resources
    ret = dos_pd_alloc_res(pd_dosp);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("PD-DoS::{}: Unable to alloc. resources",
                      __FUNCTION__);
        goto end;
    }

    // Program HW
    ret = dos_pd_program_hw(pd_dosp, TRUE);

end:
    if (ret != HAL_RET_OK) {
        dos_pd_cleanup(pd_dosp);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD DoS Update
//-----------------------------------------------------------------------------
hal_ret_t
pd_dos_policy_update (pd_dos_policy_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;; 
    pd_dos_policy_t   *pd_dosp;

    HAL_TRACE_DEBUG("pd-dos:{}: updating pd state ",
                    __FUNCTION__);

    pd_dosp = (pd_dos_policy_t *)args->clone_policy->pd;
    ret = dos_pd_program_hw(pd_dosp, FALSE);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("pd-dos:{}: unable to program hw, ret : {}",
                      __FUNCTION__, ret);
    }

    return ret;

#if 0
    pd_dosp = (pd_dos_policy_t *) args->dos_policy->pd;
    // Cache the PI pointer since the ptr in the
    // args is a local copy
    void *cached_pi_ptr = pd_dosp->pi_dos_policy;
     
    pd_dosp->pi_dos_policy = (void *) args->dos_policy; 
    // Program HW
    ret = dos_pd_program_hw(pd_dosp, FALSE);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("PD-DoS::{}: Unable to program hw, ret : {}",
                      __FUNCTION__, ret);
    }
    // Revert back to the cached PI ptr
    pd_dosp->pi_dos_policy = cached_pi_ptr;
#endif
}

//-----------------------------------------------------------------------------
// PD DoS Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_dos_policy_delete (pd_dos_policy_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_dos_policy_t  *dos_pd;

    HAL_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->dos_policy != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->dos_policy->pd != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("pd-dos:{}:deleting pd state for dos policy handle {}",
                    __FUNCTION__, args->dos_policy->hal_handle);
    dos_pd = (pd_dos_policy_t *)args->dos_policy->pd;

    // deprogram HW
    ret = dos_pd_deprogram_hw(dos_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-dos:{}:unable to deprogram hw", __FUNCTION__);
    }

    // dealloc resources and free
    ret = dos_pd_cleanup(dos_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-dos:{}:failed pd dos delete",
                      __FUNCTION__);
    }

    return ret;
}

hal_ret_t
dos_pd_program_ddos_src_vf_tcam (uint16_t slport, int actionid,
                                 uint16_t policer_idx,
                                 p4pd_table_id tbl_id, int *idx)
{
    hal_ret_t                   ret = HAL_RET_OK;
    ddos_src_vf_swkey_t         key;
    ddos_src_vf_swkey_mask_t    mask;
    ddos_src_vf_actiondata      data;
    Tcam                        *tcam = NULL;
    uint32_t                    ret_idx;

    tcam = g_hal_state_pd->tcam_table(tbl_id);
    HAL_ASSERT(tcam != NULL);
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));
    
    key.entry_inactive_ddos_src_vf = 0;
    key.control_metadata_src_lport = slport;
    mask.entry_inactive_ddos_src_vf_mask = 0xFF;
    mask.control_metadata_src_lport_mask = 0xFFFF;
    
    data.actionid = actionid;
    data.ddos_src_vf_action_u.ddos_src_vf_ddos_src_vf_hit.ddos_src_vf_base_policer_idx = policer_idx;
    
    ret = tcam->insert(&key, &mask, &data, &ret_idx);
    if (ret == HAL_RET_DUP_INS_FAIL) {
        /* Entry already exists. Can be skipped */
        *idx = -1;
    } else {
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("DDoS src-vf tcam write failure, "
                          "idx : {}, err : {}", ret_idx, ret);
            return ret;
        }
    }
    HAL_TRACE_DEBUG("DDoS src-vf tcam write, "
                    "idx : {}, ret: {}", ret_idx, ret);
    *idx = (int) ret_idx;
    return ret;
}

hal_ret_t
dos_pd_program_ddos_service_tcam (ip_addr_t *ip_addr,
                                  uint16_t dport, uint8_t proto,
                                  uint16_t vrf, int actionid,
                                  uint16_t policer_idx, bool v4_addr,
                                  p4pd_table_id tbl_id, int *idx)
{
    hal_ret_t                   ret = HAL_RET_OK;
    ddos_service_swkey_t        key;
    ddos_service_swkey_mask_t   mask;
    ddos_service_actiondata     data;
    Tcam                        *tcam = NULL;
    uint32_t                    ret_idx;

    tcam = g_hal_state_pd->tcam_table(tbl_id);
    HAL_ASSERT(tcam != NULL);
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));
    
    if (v4_addr) {
        memcpy(key.flow_lkp_metadata_lkp_dst, &ip_addr->addr.v4_addr,
               sizeof(ipv4_addr_t));
        memset(&mask.flow_lkp_metadata_lkp_dst_mask, 0xFF,
               sizeof(ipv4_addr_t));
    } else {
        memcpy(key.flow_lkp_metadata_lkp_dst, &ip_addr->addr.v6_addr,
               sizeof(ipv6_addr_t));
        memset(&mask.flow_lkp_metadata_lkp_dst_mask, 0xFF,
               sizeof(ipv6_addr_t));
    }
    key.flow_lkp_metadata_lkp_dport = dport;
    mask.flow_lkp_metadata_lkp_dport_mask = 0xFFFF;
    key.flow_lkp_metadata_lkp_proto = proto;
    mask.flow_lkp_metadata_lkp_proto_mask = 0xFF;
    key.flow_lkp_metadata_lkp_vrf = vrf;
    mask.flow_lkp_metadata_lkp_vrf_mask = 0xFFFF;
    key.entry_inactive_ddos_service = 0;
    mask.entry_inactive_ddos_service_mask = 0xFF;
     
    data.actionid = actionid;
    data.ddos_service_action_u.ddos_service_ddos_service_hit.ddos_service_base_policer_idx = policer_idx;

    ret = tcam->insert(&key, &mask, &data, &ret_idx);
    if (ret == HAL_RET_DUP_INS_FAIL) {
        /* Entry already exists. Can be skipped */
        *idx = -1;
    } else {
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("DDoS service tcam write failure, "
                          "idx : {}, err : {}", ret_idx, ret);
            return ret;
        }
    }
    HAL_TRACE_DEBUG("DDoS service tcam write, "
                    "idx : {}, ret: {}", ret_idx, ret);
    *idx = (int) ret_idx;
    return ret;
}

hal_ret_t
dos_pd_program_ddos_src_dst_tcam (ip_addr_t *src_ip_addr,
                                  ip_addr_t *dst_ip_addr,
                                  uint16_t dport, uint8_t proto,
                                  uint16_t vrf, int actionid,
                                  uint16_t policer_idx, bool v4_addr,
                                  p4pd_table_id tbl_id, int *idx)
{
    hal_ret_t                   ret = HAL_RET_OK;
    ddos_src_dst_swkey_t        key;
    ddos_src_dst_swkey_mask_t   mask;
    ddos_src_dst_actiondata     data;
    Tcam                        *tcam = NULL;
    uint32_t                    ret_idx;

    tcam = g_hal_state_pd->tcam_table(tbl_id);
    HAL_ASSERT(tcam != NULL);
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));
    
    if (v4_addr) {
        memcpy(key.flow_lkp_metadata_lkp_src, &src_ip_addr->addr.v4_addr,
               sizeof(ipv4_addr_t));
        memset(&mask.flow_lkp_metadata_lkp_src_mask, 0xFF,
               sizeof(ipv4_addr_t));
        memcpy(key.flow_lkp_metadata_lkp_dst, &dst_ip_addr->addr.v4_addr,
               sizeof(ipv4_addr_t));
        memset(&mask.flow_lkp_metadata_lkp_dst_mask, 0xFF,
               sizeof(ipv4_addr_t));
    } else {
        memcpy(key.flow_lkp_metadata_lkp_src, &src_ip_addr->addr.v6_addr,
               sizeof(ipv6_addr_t));
        memset(&mask.flow_lkp_metadata_lkp_src_mask, 0xFF,
               sizeof(ipv6_addr_t));
        memcpy(key.flow_lkp_metadata_lkp_dst, &dst_ip_addr->addr.v6_addr,
               sizeof(ipv6_addr_t));
        memset(&mask.flow_lkp_metadata_lkp_dst_mask, 0xFF,
               sizeof(ipv6_addr_t));
    }
    key.flow_lkp_metadata_lkp_dport = dport;
    mask.flow_lkp_metadata_lkp_dport_mask = 0xFFFF;
    key.flow_lkp_metadata_lkp_proto = proto;
    mask.flow_lkp_metadata_lkp_proto_mask = 0xFF;
    key.flow_lkp_metadata_lkp_vrf = vrf;
    mask.flow_lkp_metadata_lkp_vrf_mask = 0xFFFF;
    key.entry_inactive_ddos_src_dst = 0;
    mask.entry_inactive_ddos_src_dst_mask = 0xFF;
    
    data.actionid = actionid;
    data.ddos_src_dst_action_u.ddos_src_dst_ddos_src_dst_hit.ddos_src_dst_base_policer_idx = policer_idx;

    ret = tcam->insert(&key, &mask, &data, &ret_idx);
    if (ret == HAL_RET_DUP_INS_FAIL) {
        /* Entry already exists. Can be skipped */
        *idx = -1;
    } else {
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("DDoS service tcam write failure, "
                          "idx : {}, err : {}", ret_idx, ret);
            return ret;
        }
    }

    HAL_TRACE_DEBUG("DDoS service tcam write, "
                    "idx : {}, ret: {}", ret_idx, ret);
    *idx = (int) ret_idx;
    return ret;
}

#define DDOS_POLICER(_arg) d.ddos_service_policer_action_u.ddos_service_policer_execute_ddos_service_policer._arg

hal_ret_t
dos_pd_program_ddos_policer (uint8_t actionid, bool pps,
                             bool color_aware, uint32_t cir, uint32_t cbr,
                             uint32_t pir, uint32_t pbr,
                             p4pd_table_id tbl_id, uint32_t *idx)
{
    hal_ret_t                           ret = HAL_RET_OK;
    DirectMap                           *dm;
    ddos_service_policer_actiondata     d = { 0 };
    
    dm = g_hal_state_pd->dm_table(tbl_id);
    HAL_ASSERT(dm != NULL);
    
    DDOS_POLICER(entry_valid) = 1;
    DDOS_POLICER(pkt_rate) = (pps) ? 1 : 0;
    DDOS_POLICER(color_aware) = (color_aware) ? 1 : 0;
    memcpy(DDOS_POLICER(rate), &cir, sizeof(uint32_t));
    memcpy(DDOS_POLICER(burst), &cbr, sizeof(uint32_t));

    DDOS_POLICER(entry_valid2) = 1;
    DDOS_POLICER(pkt_rate2) = (pps) ? 1 : 0;
    DDOS_POLICER(color_aware2) = (color_aware) ? 1 : 0;
    memcpy(DDOS_POLICER(rate2), &pir, sizeof(uint32_t));
    memcpy(DDOS_POLICER(burst2), &pbr, sizeof(uint32_t));
    
    ret = dm->insert(&d, idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("ddos policer table write failure, tbl_id: {} err : {}",
                       tbl_id, ret);
        return ret;
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
hal_ret_t
dos_pd_program_hw(pd_dos_policy_t *pd_nw, bool create)
{
    hal_ret_t            ret = HAL_RET_OK;

    // Program DDoS service table
    // Program DDoS src_vrf table
    // Program DDoS src-dst table

    // Program DDoS service policer
    // Program DDoS src_vrf policer
    // Program DDoS src-dst policer

    return ret;
}

// ----------------------------------------------------------------------------
// DeProgram HW
// ----------------------------------------------------------------------------
hal_ret_t
dos_pd_deprogram_hw (pd_dos_policy_t *dos_pd)
{
    hal_ret_t            ret = HAL_RET_OK;

    // De-program DDoS service table
    // De-program DDoS src_vrf table
    // De-program DDoS src-dst table

    // De-program DDoS service policer
    // De-program DDoS src_vrf policer
    // De-program DDoS src-dst policer
    // De-program Input properties Table

    return ret;
}

// ----------------------------------------------------------------------------
// Allocate resources for PD Uplink if
// ----------------------------------------------------------------------------
hal_ret_t 
dos_pd_alloc_res(pd_dos_policy_t *pd_nw)
{
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// De-Allocate resources for PD Uplink if
// ----------------------------------------------------------------------------
hal_ret_t 
dos_pd_dealloc_res(pd_dos_policy_t *pd_nw)
{
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// PD Cleanup
//  - Release all resources
//  - Delink PI <-> PD
//  - Free PD Tenant
//  Note:
//      - Just free up whatever PD has. 
//      - Dont use this inplace of delete. Delete may result in giving callbacks
//        to others.
//-----------------------------------------------------------------------------
hal_ret_t
dos_pd_cleanup(pd_dos_policy_t *dos_pd)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!dos_pd) {
        // Nothing to do
        goto end;
    }

    // Releasing resources
    ret = dos_pd_dealloc_res(dos_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-dos:{}: unable to dealloc res for dos hdl: {}", 
                      __FUNCTION__, 
                      ((dos_policy_t*)(dos_pd->pi_dos_policy))->hal_handle);
        goto end;
    }

    // Delinking PI<->PD
    dos_delink_pi_pd(dos_pd, (dos_policy_t *)dos_pd->pi_dos_policy);

    // Freeing PD
    dos_pd_free(dos_pd);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
dos_link_pi_pd(pd_dos_policy_t *pd_nw, dos_policy_t *pi_nw)
{
    pd_nw->pi_dos_policy = pi_nw;
    dos_set_pd_dos(pi_nw, pd_nw);
}

// ----------------------------------------------------------------------------
// Un-Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
dos_delink_pi_pd(pd_dos_policy_t *pd_nw, dos_policy_t  *pi_nw)
{
    pd_nw->pi_dos_policy = NULL;
    dos_set_pd_dos(pi_nw, NULL);
}

// ----------------------------------------------------------------------------
// Makes a clone
// ----------------------------------------------------------------------------
hal_ret_t
pd_dos_policy_make_clone(dos_policy_t *dosp, dos_policy_t *clone)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_dos_policy_t     *pd_dosp_clone = NULL;

    pd_dosp_clone = dos_pd_alloc_init();
    if (pd_dosp_clone == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    memcpy(pd_dosp_clone, dosp->pd, sizeof(pd_dos_policy_t));

    dos_link_pi_pd(pd_dosp_clone, clone);

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Frees PD memory without indexer free.
// ----------------------------------------------------------------------------
hal_ret_t
pd_dos_policy_mem_free(pd_dos_policy_args_t *args)
{
    pd_dos_policy_t       *dos_pd;
    hal_ret_t             ret = HAL_RET_OK;

    dos_pd = (pd_dos_policy_t *)args->dos_policy->pd;
    dos_pd_mem_free(dos_pd);

    return ret;
}

}    // namespace pd
}    // namespace hal

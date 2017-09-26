#include <hal_lock.hpp>
#include <hal_state_pd.hpp>
#include <pd_api.hpp>
#include <interface_api.hpp>
#include <l2segment_api.hpp>
#include <interface.pb.h>
#include <if_pd.hpp>
#include <tunnelif_pd.hpp>
#include <defines.h>
#include "l2seg_pd.hpp"
#include "if_pd_utils.hpp"

namespace hal {
namespace pd {

// ----------------------------------------------------------------------------
// Tunnel If Create
// ----------------------------------------------------------------------------
hal_ret_t 
pd_tunnelif_create(pd_if_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;; 
    pd_tunnelif_t        *pd_tunnelif;

    HAL_TRACE_DEBUG("PD-TUNNELIF::{}: Creating pd state for Tunnelif: {}", 
                    __FUNCTION__, if_get_if_id(args->intf));

    // Create Tunnel If PD
    pd_tunnelif = pd_tunnelif_alloc_init();
    if (pd_tunnelif == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    link_pi_pd(pd_tunnelif, args->intf);

    // Allocate Resources
    ret = pd_tunnelif_alloc_res(pd_tunnelif);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("PD-TUNNELIF::{}: Unable to alloc. resources for TunnelIf: {}",
                      __FUNCTION__, if_get_if_id(args->intf));
        goto end;
    }

    // Program HW
    ret = pd_tunnelif_program_hw(pd_tunnelif);

end:
    if (ret != HAL_RET_OK) {
        unlink_pi_pd(pd_tunnelif, args->intf);
        pd_tunnelif_free(pd_tunnelif);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Allocate and Initialize Tunnel IF PD Instance
// ----------------------------------------------------------------------------
pd_tunnelif_t *
pd_tunnelif_alloc_init(void)
{
    return pd_tunnelif_init(pd_tunnelif_alloc());
}

// ----------------------------------------------------------------------------
// Allocate Tunnel IF Instance
// ----------------------------------------------------------------------------
pd_tunnelif_t *
pd_tunnelif_alloc (void)
{
    pd_tunnelif_t    *tunnelif;

    tunnelif = (pd_tunnelif_t *)g_hal_state_pd->tunnelif_pd_slab()->alloc();
    if (tunnelif == NULL) {
        return NULL;
    }
    return tunnelif;
}

// ----------------------------------------------------------------------------
// Initialize Tunnel IF PD instance
// ----------------------------------------------------------------------------
pd_tunnelif_t *
pd_tunnelif_init (pd_tunnelif_t *tunnelif)
{
    if (!tunnelif) {
        return NULL;
    }
    for (int i = 0; i < 3; i++) {
        tunnelif->imn_idx[i] = -1;
        tunnelif->imt_idx[i] = -1;
    }
    tunnelif->tunnel_rw_idx = -1;
    return tunnelif;
}

// ----------------------------------------------------------------------------
// Allocate resources for PD Tunnel IF
// ----------------------------------------------------------------------------
hal_ret_t 
pd_tunnelif_alloc_res(pd_tunnelif_t *pd_tunnelif)
{
    /*
     * Nothing to do here since we try to program the HW directly and rollback
     * the state if hw programming fails due to any reason
     */
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
hal_ret_t
pd_tunnelif_program_hw(pd_tunnelif_t *pd_tunnelif)
{
    hal_ret_t            ret;

    // Program Input mapping native table
    ret = pd_tunnelif_pgm_tunnel_rewrite_tbl(pd_tunnelif);
    if (ret != HAL_RET_OK)
        goto fail_flag;
    ret = pd_tunnelif_pgm_inp_mapping_native_tbl(pd_tunnelif);
    if ((ret != HAL_RET_OK) && (ret != HAL_RET_DUP_INS_FAIL))
        goto fail_flag;
    ret = pd_tunnelif_pgm_inp_mapping_tunneled_tbl(pd_tunnelif);
    if ((ret != HAL_RET_OK) && (ret != HAL_RET_DUP_INS_FAIL))
        goto fail_flag;
    
    return HAL_RET_OK;

fail_flag:
    HAL_TRACE_DEBUG("ERROR: pd_tunnelif_program_hw");
    return (pd_tunnelif_deprogram_hw(pd_tunnelif));
}

// ----------------------------------------------------------------------------
// De-Program HW
// ----------------------------------------------------------------------------
hal_ret_t
pd_tunnelif_deprogram_hw(pd_tunnelif_t *pd_tunnelif)
{
    hal_ret_t            ret;
    /* Deprogram input mapping native */
    ret = pd_tunnelif_del_inp_mapp_entries(pd_tunnelif,
                                           P4TBL_ID_INPUT_MAPPING_NATIVE);
    /* Deprogram input mapping tunneled */
    ret = pd_tunnelif_del_inp_mapp_entries(pd_tunnelif,
                                           P4TBL_ID_INPUT_MAPPING_TUNNELED);
    /* Deprogram tunnel rewrite table */
    ret = pd_tunnelif_del_tunnel_rw_table_entry(pd_tunnelif);
    return ret;
}

// ----------------------------------------------------------------------------
// Freeing TUNNELIF PD
// ----------------------------------------------------------------------------
hal_ret_t
pd_tunnelif_free (pd_tunnelif_t *tunnelif)
{
    g_hal_state_pd->tunnelif_pd_slab()->free(tunnelif);
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
link_pi_pd(pd_tunnelif_t *pd_tunnelif, if_t *pi_if)
{
    pd_tunnelif->pi_if = pi_if;
    if_set_pd_if(pi_if, pd_tunnelif);
}

// ----------------------------------------------------------------------------
// Un-Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
unlink_pi_pd(pd_tunnelif_t *pd_tunnelif, if_t *pi_if)
{
    pd_tunnelif->pi_if = NULL;
    if_set_pd_if(pi_if, NULL);
}

hal_ret_t
pd_tunnelif_del_inp_mapp_entries(pd_tunnelif_t *pd_tunnelif,
                                 p4pd_table_id tbl_id)
{
    Tcam        *tcam;
    hal_ret_t   ret;
    int         *arr;

    tcam = g_hal_state_pd->tcam_table(tbl_id);
    HAL_ASSERT(tcam != NULL);

    if (tbl_id == P4TBL_ID_INPUT_MAPPING_NATIVE) {
        arr = pd_tunnelif->imn_idx;
    } else {
        arr = pd_tunnelif->imt_idx;
    }

    for (int i = 0; i < 3; i++) {
        if (arr[i] != -1) {
            ret = tcam->remove(arr[i]);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Input mapping native tcam remove failure, "
                              "idx : {}, err : {}", arr[i], ret);
            }
            arr[i] = -1;
        }
    }
    return ret;
}

hal_ret_t
pd_tunnelif_program_tcam(ip_addr_t *ip_addr,
                         int tunnel_type, bool inner_v4_vld,
                         bool inner_v6_vld, bool v4_tep,
                         int actionid,
                         p4pd_table_id tbl_id, int *idx)
{
    hal_ret_t                           ret = HAL_RET_OK;
    input_mapping_native_swkey_t        key;
    input_mapping_native_swkey_mask_t   mask;
    input_mapping_native_actiondata     data;
    Tcam                                *tcam;
    uint32_t                            ret_idx;

    tcam = g_hal_state_pd->tcam_table(tbl_id);
    HAL_ASSERT(tcam != NULL);
    /* Input mapping native and tunneled tables have the same key, mask and data
     * So, we can populate the structs and typecast accordingly */
    memset(&key, 0, sizeof(input_mapping_native_swkey_t));
    memset(&mask, 0, sizeof(input_mapping_native_swkey_mask_t));
    key.inner_ipv4_valid = inner_v4_vld;
    key.inner_ipv6_valid = inner_v6_vld;
    key.tunnel_metadata_tunnel_type = tunnel_type;
    mask.inner_ipv4_valid_mask = 0xFF;
    mask.inner_ipv6_valid_mask = 0xFF;
    mask.tunnel_metadata_tunnel_type_mask = 0xFF;

    if (v4_tep) {
        key.ipv4_valid = 1;
        key.input_mapping_native_u1.ipv4_dstAddr = ip_addr->addr.v4_addr;
        mask.ipv4_valid_mask = 0xFF;
        mask.input_mapping_native_mask_u1.ipv4_dstAddr_mask = 0xFFFFFFFF;
    } else {
        key.ipv6_valid = 1;
        memcpy(key.input_mapping_native_u1.ipv6_dstAddr,
               ip_addr->addr.v6_addr.addr8, IP6_ADDR8_LEN);
        memrev(key.input_mapping_native_u1.ipv6_dstAddr, IP6_ADDR8_LEN);
        mask.ipv6_valid_mask = 0xFF;
        memset(mask.input_mapping_native_mask_u1.ipv6_dstAddr_mask, 0xFF, IP6_ADDR8_LEN);
    }
    data.actionid = actionid;
    ret = tcam->insert(&key, &mask, &data, &ret_idx);
    if (ret == HAL_RET_DUP_INS_FAIL) {
        /* Entry already exists. Can be skipped */
        *idx = -1;
    } else {
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Input mapping table tcam write failure, "
                          "idx : {}, err : {}", ret_idx, ret);
            return ret;
        }
    }
    HAL_TRACE_DEBUG("Input mapping table tcam write, "
                    "idx : {}, ret: {}", ret_idx, ret);
    *idx = (int) ret_idx;
    return ret;
}

hal_ret_t
pd_tunnelif_pgm_inp_mapping_native_tbl(pd_tunnelif_t *pd_tunnelif)
{
    int                                  idx;
    hal_ret_t                            ret;
    if_t                                 *pi_if;
    bool                                 v4_tep = false;

    pi_if = (if_t *) pd_tunnelif->pi_if;
    if (!pi_if) {
        return (HAL_RET_ERR);
    }
    if (pi_if->vxlan_ltep.af == IP_AF_IPV4) {
        v4_tep = true;
    }
    
    /* We program 3 entries for every MyTEP */
    /* Entry 1 */
    ret = pd_tunnelif_program_tcam(&pi_if->vxlan_ltep, INGRESS_TUNNEL_TYPE_VXLAN,
                               true, false, v4_tep, INPUT_MAPPING_NATIVE_NOP_ID,
                               P4TBL_ID_INPUT_MAPPING_NATIVE, &idx);
    if ((ret != HAL_RET_OK) && (ret != HAL_RET_DUP_INS_FAIL))
        goto fail_flag;
    pd_tunnelif->imn_idx[0] = idx;
    /* Entry 2 */
    ret = pd_tunnelif_program_tcam(&pi_if->vxlan_ltep, INGRESS_TUNNEL_TYPE_VXLAN,
                               false, true, v4_tep, INPUT_MAPPING_NATIVE_NOP_ID,
                               P4TBL_ID_INPUT_MAPPING_NATIVE, &idx);
    if ((ret != HAL_RET_OK) && (ret != HAL_RET_DUP_INS_FAIL))
        goto fail_flag;
    pd_tunnelif->imn_idx[1] = idx;
    /* Entry 3 */
    ret = pd_tunnelif_program_tcam(&pi_if->vxlan_ltep, INGRESS_TUNNEL_TYPE_VXLAN,
                               false, false, v4_tep, INPUT_MAPPING_NATIVE_NOP_ID,
                               P4TBL_ID_INPUT_MAPPING_NATIVE, &idx);
    if ((ret != HAL_RET_OK) && (ret != HAL_RET_DUP_INS_FAIL))
        goto fail_flag;
    pd_tunnelif->imn_idx[2] = idx;

    return HAL_RET_OK;

fail_flag:
    ret = pd_tunnelif_del_inp_mapp_entries(pd_tunnelif,
                                           P4TBL_ID_INPUT_MAPPING_NATIVE);
    return ret;
}

hal_ret_t
pd_tunnelif_pgm_inp_mapping_tunneled_tbl(pd_tunnelif_t *pd_tunnelif)
{
    int                                  idx;
    hal_ret_t                            ret;
    if_t                                 *pi_if;
    bool                                 v4_tep = false;

    pi_if = (if_t *) pd_tunnelif->pi_if;
    if (!pi_if) {
        return (HAL_RET_ERR);
    }
    if (pi_if->vxlan_ltep.af == IP_AF_IPV4) {
        v4_tep = true;
    }
    
    /* We program 3 entries for every MyTEP */
    /* Entry 1 */
    ret = pd_tunnelif_program_tcam(&pi_if->vxlan_ltep, INGRESS_TUNNEL_TYPE_VXLAN,
                               true, false, v4_tep,
                               INPUT_MAPPING_TUNNELED_TUNNELED_IPV4_PACKET_ID,
                               P4TBL_ID_INPUT_MAPPING_TUNNELED, &idx);
    if ((ret != HAL_RET_OK) && (ret != HAL_RET_DUP_INS_FAIL))
        goto fail_flag;
    pd_tunnelif->imt_idx[0] = idx;
    /* Entry 2 */
    ret = pd_tunnelif_program_tcam(&pi_if->vxlan_ltep, INGRESS_TUNNEL_TYPE_VXLAN,
                               false, true, v4_tep,
                               INPUT_MAPPING_TUNNELED_TUNNELED_IPV6_PACKET_ID,
                               P4TBL_ID_INPUT_MAPPING_TUNNELED, &idx);
    if ((ret != HAL_RET_OK) && (ret != HAL_RET_DUP_INS_FAIL))
        goto fail_flag;
    pd_tunnelif->imt_idx[1] = idx;
    /* Entry 3 */
    ret = pd_tunnelif_program_tcam(&pi_if->vxlan_ltep, INGRESS_TUNNEL_TYPE_VXLAN,
                               false, false, v4_tep,
                               INPUT_MAPPING_TUNNELED_TUNNELED_NON_IP_PACKET_ID,
                               P4TBL_ID_INPUT_MAPPING_TUNNELED, &idx);
    if ((ret != HAL_RET_OK) && (ret != HAL_RET_DUP_INS_FAIL))
        goto fail_flag;
    pd_tunnelif->imt_idx[2] = idx;

    return HAL_RET_OK;

fail_flag:
    ret = pd_tunnelif_del_inp_mapp_entries(pd_tunnelif,
                                           P4TBL_ID_INPUT_MAPPING_TUNNELED);
    return ret;
}

hal_ret_t
pd_tunnelif_add_tunnel_rw_table_entry (pd_tunnelif_t *pd_tif, uint8_t actionid,
                                       tunnel_rewrite_action_union_t *act)
{
    hal_ret_t                   ret;
    DirectMap                   *dm;
    tunnel_rewrite_actiondata   d = { 0 };
    uint32_t                    idx;
    
    dm = g_hal_state_pd->dm_table(P4TBL_ID_TUNNEL_REWRITE);
    HAL_ASSERT(dm != NULL);
    HAL_ASSERT(pd_tif != NULL);
    
    d.actionid = actionid;
    d.tunnel_rewrite_action_u = *act;
    
    // insert the entry
    ret = dm->insert(&d, &idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel rewrite table write failure, err : {}", ret);
        return ret;
    }
    pd_tif->tunnel_rw_idx = idx;
    return ret;
}

hal_ret_t
pd_tunnelif_del_tunnel_rw_table_entry (pd_tunnelif_t *pd_tif)
{
    hal_ret_t   ret;
    DirectMap   *dm;

    dm = g_hal_state_pd->dm_table(P4TBL_ID_TUNNEL_REWRITE);
    HAL_ASSERT(dm != NULL);
    HAL_ASSERT(pd_tif != NULL);
    
    // remove the entry
    if (pd_tif->tunnel_rw_idx != -1) {
        ret = dm->remove(pd_tif->tunnel_rw_idx);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("tunnel rewrite table write failure, err : {}", ret);
            return ret;
        }
        pd_tif->tunnel_rw_idx = -1;
    }
    return ret;
}


tunnel_rewrite_actions_en
pd_tunnelif_get_p4pd_encap_action_id (intf::IfTunnelEncapType encap_type)
{
    switch (encap_type) {
        case intf::IfTunnelEncapType::IF_TUNNEL_ENCAP_TYPE_VXLAN:
            return TUNNEL_REWRITE_ENCAP_VXLAN_ID;
        default:
            return TUNNEL_REWRITE_NOP_ID;
    }
    return TUNNEL_REWRITE_NOP_ID;
}

hal_ret_t
pd_tunnelif_pgm_tunnel_rewrite_tbl(pd_tunnelif_t *pd_tif)
{
    ep_t *remote_tep_ep;
    if_t *pi_if;
    l2seg_t *l2seg;
    tunnel_rewrite_action_union_t act;
    uint8_t actionid;
    if_t *ep_if;
    uint8_t vlan_v;
    uint16_t vlan_id;
    mac_addr_t *mac;
    hal_ret_t ret = HAL_RET_OK;
    bool v4_valid = FALSE;

    memset(&act, 0, sizeof(tunnel_rewrite_action_union_t));
    pi_if = (if_t *) pd_tif->pi_if;
    HAL_ASSERT(pi_if);
    
    actionid = pd_tunnelif_get_p4pd_encap_action_id (pi_if->encap_type);
    
    remote_tep_ep = if_get_tunnelif_remote_tep_ep(pi_if, &v4_valid);
    HAL_ASSERT(remote_tep_ep);
    
    l2seg = find_l2seg_by_handle(remote_tep_ep->l2seg_handle);
    HAL_ASSERT(l2seg);
    
    ep_if = find_if_by_handle(remote_tep_ep->if_handle);
    HAL_ASSERT(ep_if);
    
    ret = if_l2seg_get_encap(ep_if, l2seg, &vlan_v, &vlan_id);
    HAL_ASSERT(ret == HAL_RET_OK);
    
    if (actionid == TUNNEL_REWRITE_ENCAP_VXLAN_ID) {
        /* Populate vxlan encap params */
        /* MAC DA */
        mac = ep_get_mac_addr(remote_tep_ep);
        memcpy(act.tunnel_rewrite_encap_vxlan.mac_da, mac, ETH_ADDR_LEN);
        memrev(act.tunnel_rewrite_encap_vxlan.mac_da, ETH_ADDR_LEN);
        /* MAC SA */
        mac = ep_get_rmac(remote_tep_ep, l2seg);
        memcpy(act.tunnel_rewrite_encap_vxlan.mac_sa, mac, ETH_ADDR_LEN);
        memrev(act.tunnel_rewrite_encap_vxlan.mac_sa, ETH_ADDR_LEN);
        
        memcpy(&act.tunnel_rewrite_encap_vxlan.ip_sa, &pi_if->vxlan_ltep.addr,
               sizeof(ipvx_addr_t));
        memcpy(&act.tunnel_rewrite_encap_vxlan.ip_da, &pi_if->vxlan_rtep.addr,
               sizeof(ipvx_addr_t));
        if (v4_valid) {
            act.tunnel_rewrite_encap_vxlan.ip_type = IP_HEADER_TYPE_IPV4;
        } else {
            HAL_TRACE_ERR("PD-TUNNELIF::{}: Invalid outer encap header",
                          __FUNCTION__);
            ret = HAL_RET_ERR;
            goto fail_flag;
#ifdef PHASE2
            act.tunnel_rewrite_encap_vxlan.ip_type = IP_HEADER_TYPE_IPV6;
            memrev(act.tunnel_rewrite_encap_vxlan.ip_sa, IP6_ADDR8_LEN);
            memrev(act.tunnel_rewrite_encap_vxlan.ip_da, IP6_ADDR8_LEN);
#endif /* PHASE2 */
        }
        act.tunnel_rewrite_encap_vxlan.vlan_valid = vlan_v;
        act.tunnel_rewrite_encap_vxlan.vlan_id = vlan_id;
    }
    ret = pd_tunnelif_add_tunnel_rw_table_entry (pd_tif, actionid, &act);
    if (ret != HAL_RET_OK)
        goto fail_flag;
    
    return ret;

fail_flag:
    return ret;
}


}    // namespace pd
}    // namespace hal

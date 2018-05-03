#include "nic/include/hal_lock.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/include/l2segment_api.hpp"
#include "nic/gen/proto/hal/interface.pb.h"
#include "nic/hal/pd/iris/nw/if_pd.hpp"
#include "nic/hal/pd/iris/nw/tunnelif_pd.hpp"
#include "nic/p4/iris/include/defines.h"
#include "l2seg_pd.hpp"
#include "nic/hal/pd/iris/nw/if_pd_utils.hpp"

namespace hal {
namespace pd {

// ----------------------------------------------------------------------------
// Tunnel If Create
// ----------------------------------------------------------------------------
hal_ret_t
pd_tunnelif_create(pd_if_create_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;;
    pd_tunnelif_t        *pd_tunnelif;

    HAL_TRACE_DEBUG("Creating pd state for Tunnelif: {}",
                    if_get_if_id(args->intf));

    // Create Tunnel If PD
    pd_tunnelif = pd_tunnelif_alloc_init();
    if (pd_tunnelif == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    pd_tunnelif_link_pi_pd(pd_tunnelif, args->intf);

    // Allocate Resources
    ret = pd_tunnelif_alloc_res(pd_tunnelif);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("unable to alloc. resources for TunnelIf: {}",
                      if_get_if_id(args->intf));
        goto end;
    }

    // Program HW
    ret = pd_tunnelif_program_hw(pd_tunnelif);

end:
    if (ret != HAL_RET_OK) {
        pd_tunnelif_cleanup(pd_tunnelif);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD TunnelIf Update
//-----------------------------------------------------------------------------
hal_ret_t
pd_tunnelif_update (pd_if_update_args_t *args)
{
    // Nothing to do for now
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// PD Tunnelif Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_tunnelif_delete (pd_if_delete_args_t *args)
{
    hal_ret_t        ret = HAL_RET_OK;
    pd_tunnelif_t    *tunnelif_pd;

    HAL_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->intf != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->intf->pd_if != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("deleting pd state for tunnelif: {}",
                    args->intf->if_id);
    tunnelif_pd = (pd_tunnelif_t *)args->intf->pd_if;

    // deprogram HW
    ret = pd_tunnelif_deprogram_hw(tunnelif_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to deprogram hw");
    }

    ret = pd_tunnelif_cleanup(tunnelif_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed pd tunnelif delete");
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD Tunnelif Get
//-----------------------------------------------------------------------------
hal_ret_t
pd_tunnelif_get (pd_if_get_args_t *args)
{
    hal_ret_t               ret = HAL_RET_OK;
    if_t                    *hal_if = args->hal_if;
    pd_tunnelif_t           *tunnelif_pd = (pd_tunnelif_t *)hal_if->pd_if;
    InterfaceGetResponse    *rsp = args->rsp;

    auto tnnl_info = rsp->mutable_status()->mutable_tunnel_info();
    for (int i = 0; i < 3; i++) {
        tnnl_info->add_inp_map_nat_idx(tunnelif_pd->imn_idx[i]);
        tnnl_info->add_inp_map_tnl_idx(tunnelif_pd->imt_idx[i]);
    }
    tnnl_info->set_tunnel_rw_idx(tunnelif_pd->tunnel_rw_idx);

    return ret;
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
// De-Allocate resources for PD Tunnel IF
// ----------------------------------------------------------------------------
hal_ret_t
pd_tunnelif_dealloc_res(pd_tunnelif_t *pd_tunnelif)
{
    /*
     * Nothing to do here since we try to program the HW directly and rollback
     * the state if hw programming fails due to any reason
     */
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// PD TunnelIf Cleanup
//  - Release all resources
//  - Delink PI <-> PD
//  - Free PD If
//  Note:
//      - Just free up whatever PD has.
//      - Dont use this inplace of delete. Delete may result in giving callbacks
//        to others.
//-----------------------------------------------------------------------------
hal_ret_t
pd_tunnelif_cleanup(pd_tunnelif_t *pd_tunnelif)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!pd_tunnelif) {
        // Nothing to do
        goto end;
    }

    // Releasing resources
    ret = pd_tunnelif_dealloc_res(pd_tunnelif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to dealloc res for enicif: {}",
                      ((if_t *)(pd_tunnelif->pi_if))->if_id);
        goto end;
    }

    // Delinking PI<->PD
    pd_tunnelif_delink_pi_pd(pd_tunnelif, (if_t *)pd_tunnelif->pi_if);

    // Freeing PD
    pd_tunnelif_free(pd_tunnelif);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
hal_ret_t
pd_tunnelif_program_hw(pd_tunnelif_t *pd_tunnelif)
{
    hal_ret_t            ret;
    if_t                 *hal_if;

    hal_if = (if_t *) pd_tunnelif->pi_if;
    HAL_ASSERT(hal_if != NULL);

    ret = pd_tunnelif_pgm_tunnel_rewrite_tbl(pd_tunnelif);
    if (ret != HAL_RET_OK)
        goto fail_flag;

    /* Tunnel termination required only for vxlan */
    if (hal_if->encap_type ==
            intf::IfTunnelEncapType::IF_TUNNEL_ENCAP_TYPE_VXLAN) {
        // Program Input mapping native table
        ret = pd_tunnelif_pgm_inp_mapping_native_tbl(pd_tunnelif);
        if ((ret != HAL_RET_OK) && (ret != HAL_RET_DUP_INS_FAIL))
            goto fail_flag;
        // Program Input mapping tunneled table
        ret = pd_tunnelif_pgm_inp_mapping_tunneled_tbl(pd_tunnelif);
        if ((ret != HAL_RET_OK) && (ret != HAL_RET_DUP_INS_FAIL))
            goto fail_flag;
    }

    return HAL_RET_OK;

fail_flag:
    HAL_TRACE_ERR("unable to program hw");
    return ret;
    // return (pd_tunnelif_deprogram_hw(pd_tunnelif));
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
    ret = pd_tunnelif_depgm_tunnel_rewrite_tbl(pd_tunnelif);
    return ret;
}

hal_ret_t
pd_tunnelif_del_inp_mapp_entries(pd_tunnelif_t *pd_tunnelif,
                                 p4pd_table_id tbl_id)
{
    tcam        *tcam;
    hal_ret_t   ret;
    sdk_ret_t   sdk_ret;
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
            sdk_ret = tcam->remove(arr[i]);
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
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
    sdk_ret_t                           sdk_ret;
    input_mapping_native_swkey_t        key;
    input_mapping_native_swkey_mask_t   mask;
    input_mapping_native_actiondata     data;
    tcam                                *tcam;
    uint32_t                            ret_idx = 0;

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
    sdk_ret = tcam->insert(&key, &mask, &data, &ret_idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
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
    sdk_ret_t                   sdk_ret;
    directmap                   *dm;
    tunnel_rewrite_actiondata   d = { 0 };
    uint32_t                    idx;

    dm = g_hal_state_pd->dm_table(P4TBL_ID_TUNNEL_REWRITE);
    HAL_ASSERT(dm != NULL);
    HAL_ASSERT(pd_tif != NULL);

    d.actionid = actionid;
    d.tunnel_rewrite_action_u = *act;

    // insert the entry
    sdk_ret = dm->insert(&d, &idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel rewrite table write failure, err : {}", ret);
        return ret;
    }
    pd_tif->tunnel_rw_idx = idx;
    return ret;
}

hal_ret_t
pd_tunnelif_depgm_tunnel_rewrite_tbl(pd_tunnelif_t *pd_tif)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd_tnnl_rw_entry_key_t      key = { 0 };

    ret = pd_tunnelif_form_data(&key, pd_tif);
    HAL_ASSERT(ret == HAL_RET_OK);

    ret = tnnl_rw_entry_delete(&key);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to deprogram tnnl rw table: ret:{}", ret);
    } else {
        HAL_TRACE_DEBUG("deprogrammed tnnl rw table. index:{}",
                        pd_tif->tunnel_rw_idx);
    }
    pd_tif->tunnel_rw_idx = INVALID_INDEXER_INDEX;

    return ret;
}

hal_ret_t
pd_tunnelif_del_tunnel_rw_table_entry (pd_tunnelif_t *pd_tif)
{
    hal_ret_t   ret;
    sdk_ret_t   sdk_ret;
    directmap   *dm;

    dm = g_hal_state_pd->dm_table(P4TBL_ID_TUNNEL_REWRITE);
    HAL_ASSERT(dm != NULL);
    HAL_ASSERT(pd_tif != NULL);

    // remove the entry
    if (pd_tif->tunnel_rw_idx != -1) {
        sdk_ret = dm->remove(pd_tif->tunnel_rw_idx);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
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
        case intf::IfTunnelEncapType::IF_TUNNEL_ENCAP_TYPE_GRE:
            return TUNNEL_REWRITE_ENCAP_ERSPAN_ID;
        default:
            return TUNNEL_REWRITE_NOP_ID;
    }
    return TUNNEL_REWRITE_NOP_ID;
}

hal_ret_t
pd_tunnelif_form_data (pd_tnnl_rw_entry_key_t *tnnl_rw_key,
                       pd_tunnelif_t *pd_tif)
{
    hal_ret_t   ret            = HAL_RET_OK;
    if_t        *pi_if         = NULL;
    l2seg_t     *l2seg         = NULL;
    if_t        *ep_if         = NULL;
    mac_addr_t  *mac           = NULL;
    ep_t        *rtep_ep = NULL;
    uint8_t     actionid;
    uint8_t     vlan_v;
    uint16_t    vlan_id;

    memset(tnnl_rw_key, 0, sizeof(pd_tnnl_rw_entry_key_t));

    pi_if = (if_t *) pd_tif->pi_if;
    HAL_ABORT_TRACE(pi_if, "PD should always have PI");

    actionid = pd_tunnelif_get_p4pd_encap_action_id(pi_if->encap_type);

    rtep_ep = find_ep_by_handle(pi_if->rtep_ep_handle);
    HAL_ABORT_TRACE(rtep_ep, "ABORT:should have caught in PI");

    l2seg = l2seg_lookup_by_handle(rtep_ep->l2seg_handle);
    HAL_ABORT_TRACE(l2seg, "ABORT: EP should not exist with no l2seg");

    ep_if = find_if_by_handle(rtep_ep->if_handle);
    HAL_ABORT_TRACE(ep_if, "ABORT: EP should not exist with no IF");

    ret = if_l2seg_get_encap(ep_if, l2seg, &vlan_v, &vlan_id);
    HAL_ABORT_TRACE(ret == HAL_RET_OK, "ABORT: EP presence means "
                    "l2seg should be UP on IF");

    tnnl_rw_key->tnnl_rw_act = (tunnel_rewrite_actions_en) actionid;

    if ((actionid == TUNNEL_REWRITE_ENCAP_VXLAN_ID) ||
        (actionid == TUNNEL_REWRITE_ENCAP_ERSPAN_ID)) {
        /* MAC DA */
        mac = ep_get_mac_addr(rtep_ep);
        memcpy(tnnl_rw_key->mac_da, mac, sizeof(mac_addr_t));

        /* MAC SA */
        mac = ep_get_rmac(rtep_ep, l2seg);
        memcpy(tnnl_rw_key->mac_sa, mac, sizeof(mac_addr_t));

        /* Populate vxlan encap params */
        if (actionid == TUNNEL_REWRITE_ENCAP_VXLAN_ID) {
            memcpy(&tnnl_rw_key->ip_sa, &pi_if->vxlan_ltep,
                   sizeof(ip_addr_t));
            memcpy(&tnnl_rw_key->ip_da, &pi_if->vxlan_rtep,
                   sizeof(ip_addr_t));
        } else if (actionid == TUNNEL_REWRITE_ENCAP_ERSPAN_ID) {
            memcpy(&tnnl_rw_key->ip_sa, &pi_if->gre_source,
                   sizeof(ip_addr_t));
            memcpy(&tnnl_rw_key->ip_da, &pi_if->gre_dest,
                   sizeof(ip_addr_t));
        }

        tnnl_rw_key->ip_type = IP_HEADER_TYPE_IPV4;

        tnnl_rw_key->vlan_valid = vlan_v;
        tnnl_rw_key->vlan_id = vlan_id;
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Forms the data and call lib which shares the entries
// ----------------------------------------------------------------------------
hal_ret_t
pd_tunnelif_pgm_tunnel_rewrite_tbl(pd_tunnelif_t *pd_tif)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd_tnnl_rw_entry_key_t      key = { 0 };

    ret = pd_tunnelif_form_data(&key, pd_tif);
    HAL_ASSERT(ret == HAL_RET_OK);

    ret = tnnl_rw_entry_find_or_alloc(&key, (uint32_t *)&pd_tif->tunnel_rw_idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to program tnnl rw table: ret:{}", ret);
    } else {
        HAL_TRACE_DEBUG("programmed tnnl rw table. index:{}",
                        pd_tif->tunnel_rw_idx);
    }

    return ret;
}
#if 0
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

    l2seg = l2seg_lookup_by_handle(remote_tep_ep->l2seg_handle);
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
        /*
         * Only v4 TEP addresses are supported currently. When v6-addr is added
         * for phase-2 change the size also accordingly
         * */
        memcpy(&act.tunnel_rewrite_encap_vxlan.ip_sa, &pi_if->vxlan_ltep.addr,
               sizeof(uint32_t));
        memcpy(&act.tunnel_rewrite_encap_vxlan.ip_da, &pi_if->vxlan_rtep.addr,
               sizeof(uint32_t));
        if (v4_valid) {
            act.tunnel_rewrite_encap_vxlan.ip_type = IP_HEADER_TYPE_IPV4;
        } else {
            HAL_TRACE_ERR("Invalid outer encap header");
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
#endif

// ----------------------------------------------------------------------------
// Makes a clone
// ----------------------------------------------------------------------------
hal_ret_t
pd_tunnelif_make_clone(pd_if_make_clone_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_tunnelif_t       *pd_tunnelif_clone = NULL;
    if_t *hal_if = args->hal_if;
    if_t *clone = args->clone;

    pd_tunnelif_clone = pd_tunnelif_alloc_init();
    if (pd_tunnelif_clone == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    memcpy(pd_tunnelif_clone, hal_if->pd_if, sizeof(pd_tunnelif_t));

    pd_tunnelif_link_pi_pd(pd_tunnelif_clone, clone);

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Frees PD memory without indexer free.
// ----------------------------------------------------------------------------
hal_ret_t
pd_tunnelif_mem_free(pd_if_mem_free_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_tunnelif_t  *pd_tunnif;

    pd_tunnif = (pd_tunnelif_t *)args->intf->pd_if;
    pd_tunnelif_free(pd_tunnif);

    return ret;
}
}    // namespace pd
}    // namespace hal

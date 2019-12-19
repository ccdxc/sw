// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.hpp"
#include "gen/p4gen/p4/include/p4pd.h"
#include "nic/hal/pd/iris/mcast/oif_list_pd_mgr.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/nw/if_pd.hpp"
#include "nic/hal/pd/iris/nw/if_pd_utils.hpp"
#include "nic/hal/pd/iris/nw/tnnl_rw_pd.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"

using namespace hal;

namespace hal {
namespace pd {

// Creates a new oif_list and returns handle
// hal_ret_t oif_list_create(oif_list_id_t *list)
hal_ret_t pd_oif_list_create(pd_func_args_t *pd_func_args)
{
    pd_oif_list_create_args_t *args = pd_func_args->pd_oif_list_create;
    oif_list_id_t *list = args->list;
    return g_hal_state_pd->met_table()->create_repl_list(list);
}

// Creates a contiguous block of oif_lists and returns handle to the first one
// hal_ret_t oif_list_create_block(oif_list_id_t *list, uint32_t size)
hal_ret_t pd_oif_list_create_block(pd_func_args_t *pd_func_args)
{
    pd_oif_list_create_block_args_t *args = pd_func_args->pd_oif_list_create_block;
    oif_list_id_t *list = args->list;
    uint32_t size = args->size;
    return g_hal_state_pd->met_table()->create_repl_list_block(list, size);
}

// Takes an oiflis_handle and deletes it
// hal_ret_t oif_list_delete(oif_list_id_t list)
hal_ret_t pd_oif_list_delete(pd_func_args_t *pd_func_args)
{
    pd_oif_list_delete_args_t *args = pd_func_args->pd_oif_list_delete;
    oif_list_id_t list = args->list;
    return g_hal_state_pd->met_table()->delete_repl_list(list);
}

// Takes an oiflis_handle and deletes a block starting from it
// hal_ret_t oif_list_delete_block(oif_list_id_t list, uint32_t size)
hal_ret_t pd_oif_list_delete_block(pd_func_args_t *pd_func_args)
{
    pd_oif_list_delete_block_args_t *args = pd_func_args->pd_oif_list_delete_block;
    oif_list_id_t list = args->list;
    uint32_t size = args->size;
    return g_hal_state_pd->met_table()->delete_repl_list_block(list, size);
}

// ----------------------------------------------------------------------------
// Attach an existing Replication List to another existing Replication List
// This is useful for jumping to (*, G) entries at the end of (S, G) entries
// Also helpful in jumping to all-multicast list at the end of specific lists
// ----------------------------------------------------------------------------
hal_ret_t pd_oif_list_attach(pd_func_args_t *pd_func_args)
{
    pd_oif_list_attach_args_t *args = pd_func_args->pd_oif_list_attach;
    oif_list_id_t frm = args->frm;
    oif_list_id_t to = args->to;
    return g_hal_state_pd->met_table()->attach_repl_lists(frm, to);
}

// ----------------------------------------------------------------------------
// Detach an existing Replication List from another existing Replication List
// ----------------------------------------------------------------------------
hal_ret_t pd_oif_list_detach(pd_func_args_t *pd_func_args)
{
    pd_oif_list_detach_args_t *args = pd_func_args->pd_oif_list_detach;
    oif_list_id_t frm = args->frm;
    return g_hal_state_pd->met_table()->detach_repl_lists(frm);
}

// Adds an oif to list
hal_ret_t pd_oif_list_add_oif(pd_func_args_t *pd_func_args)
{
    hal_ret_t ret;
    pd_oif_list_add_oif_args_t *args = pd_func_args->pd_oif_list_add_oif;
    oif_list_id_t list = args->list;
    oif_t *oif = args->oif;
    p4_replication_data_t data = {0};
    if_t *pi_if = NULL;
    l2seg_t *pi_l2seg = NULL;

    if (oif->intf == NULL) {
        HAL_TRACE_DEBUG("Adding drop entry to replication list: {}", list);
    } else {
        pi_if = oif->intf;
        pi_l2seg = oif->l2seg;
        ret = if_l2seg_get_multicast_rewrite_data(pi_if, pi_l2seg, NULL, &data);
        if (ret != HAL_RET_OK) {
            return ret;
        }
    }

    HAL_TRACE_DEBUG("Replication data add: isTnl: {}; isQid: {}:: rw_idx: {}:: "
                    "tnl_idx: {}:: lport : {} qtype : {} qid/vni : {}",
                    data.is_tunnel, data.is_qid, data.rewrite_index,
                    data.tunnel_rewrite_index, data.lport, data.qtype,
                    data.qid_or_vnid);

    return g_hal_state_pd->met_table()->add_replication(list, (void*)&data);
}

// Adds an rdma qp based oif to list
hal_ret_t pd_oif_list_add_qp_oif(pd_func_args_t *pd_func_args)
{
    hal_ret_t ret;
    pd_oif_list_add_qp_oif_args_t *args = pd_func_args->pd_oif_list_add_qp_oif;
    oif_list_id_t list = args->list;
    oif_t *oif = args->oif;
    uint8_t is_tagged;
    uint16_t vlan_id;
    p4_replication_data_t data = {};
    if_t *pi_if = oif->intf;
    l2seg_t *pi_l2seg = oif->l2seg;

    SDK_ASSERT_RETURN(pi_if && pi_l2seg, HAL_RET_INVALID_ARG);

    data.lport = if_get_lport_id(pi_if);

    SDK_ASSERT(hal::intf_get_if_type(pi_if) == intf::IF_TYPE_ENIC);
    hal::lif_t *lif = if_get_lif(pi_if);
    if (lif == NULL) {
        return HAL_RET_LIF_NOT_FOUND;
    }

    ret = if_l2seg_get_encap(pi_if, pi_l2seg, &is_tagged, &vlan_id);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    data.is_qid = 1;
    data.qid_or_vnid = oif->qid; // TODO refer to update_fwding_info()
    data.qtype = lif_get_qtype(lif, oif->purpose);
    data.rewrite_index = g_hal_state_pd->rwr_tbl_decap_vlan_idx();
    data.tunnel_rewrite_index = (is_tagged) ?
                                (g_hal_state_pd->tnnl_rwr_tbl_encap_vlan_idx())
                                            :
                                (0);
    HAL_TRACE_DEBUG("Replication to Enic: lif_id: {}", lif->lif_id);

    HAL_TRACE_DEBUG("Replication data: isTnl: {}:: isQid: {}:: rw_idx: {}:: "
                    "tnl_idx: {}:: lport : {} qtype : {} qid/vni : {}",
                    data.is_tunnel, data.is_qid, data.rewrite_index,
                    data.tunnel_rewrite_index, data.lport, data.qtype,
                    data.qid_or_vnid);

    return g_hal_state_pd->met_table()->add_replication(list, (void*)&data);
}

// Removes an oif from list
hal_ret_t pd_oif_list_remove_oif(pd_func_args_t *pd_func_args)
{
    hal_ret_t ret;
    pd_oif_list_remove_oif_args_t *args = pd_func_args->pd_oif_list_remove_oif;
    oif_list_id_t list = args->list;
    oif_t *oif = args->oif;
    p4_replication_data_t data = {};
    if_t *pi_if = NULL;
    l2seg_t *pi_l2seg = NULL;

    if (oif->intf == NULL) {
        HAL_TRACE_DEBUG("Removing drop entry to replication list: {}", list);
    } else {
        pi_if = oif->intf;
        pi_l2seg = oif->l2seg;
        ret = if_l2seg_get_multicast_rewrite_data(pi_if, pi_l2seg, NULL, &data);
        if (ret != HAL_RET_OK) {
            return ret;
        }
    }

    HAL_TRACE_DEBUG("Replication data del: isTnl: {}:: isQid: {}:: rw_idx: {}:: "
                    "tnl_idx: {}:: lport : {} qtype : {} qid/vni : {}",
                    data.is_tunnel, data.is_qid, data.rewrite_index,
                    data.tunnel_rewrite_index, data.lport, data.qtype,
                    data.qid_or_vnid);

    return g_hal_state_pd->met_table()->del_replication(list, (void*)&data);
}

// Check if an oif is present in the list
hal_ret_t pd_oif_list_is_member(pd_func_args_t *pd_func_args) {
    return HAL_RET_OK;
}

// Get an array of all oifs in the list
hal_ret_t pd_oif_list_get_num_oifs(pd_func_args_t *pd_func_args) {
    return HAL_RET_OK;
}

// Adds a special node for ingress driven copy
hal_ret_t pd_oif_list_set_honor_ingress(pd_func_args_t *pd_func_args)
{
    pd_oif_list_set_honor_ingress_args_t *args = pd_func_args->pd_oif_list_set_honor_ingress;
    p4_replication_data_t data = {};
    oif_list_id_t list = args->list;
    data.repl_type = TM_REPL_TYPE_HONOR_INGRESS;
    return g_hal_state_pd->met_table()->add_replication(list, (void*)&data);
}

// Deletes the special node for ingress driven copy
hal_ret_t pd_oif_list_clr_honor_ingress(pd_func_args_t *pd_func_args)
{
    pd_oif_list_clr_honor_ingress_args_t *args = pd_func_args->pd_oif_list_clr_honor_ingress;
    p4_replication_data_t data = {};
    oif_list_id_t list = args->list;
    data.repl_type = TM_REPL_TYPE_HONOR_INGRESS;
    return g_hal_state_pd->met_table()->del_replication(list, (void*)&data);
}

// Get Function for proto
hal_ret_t pd_oif_list_get(pd_func_args_t *pd_func_args) {
    pd_oif_list_get_args_t *args = pd_func_args->pd_oif_list_get;

    SDK_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->rsp != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_VERBOSE("Getting pd state for oif list {}", args->list);

    args->rsp->mutable_epd_info()->set_rep_index(args->list);
    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal


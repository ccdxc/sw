// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.h"
#include "nic/gen/iris/include/p4pd.h"
#include "nic/hal/pd/iris/oif_list_pd_mgr.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/if_pd.hpp"
#include "nic/hal/pd/iris/if_pd_utils.hpp"
#include "nic/hal/pd/iris/tnnl_rw_pd.hpp"
#include "hal_state_pd.hpp"
#include "nic/p4/nw/include/defines.h"

using namespace hal;

namespace hal {
namespace pd {

// Creates a new oif_list and returns handle
hal_ret_t oif_list_create(oif_list_id_t *list)
{
    return g_hal_state_pd->met_table()->create_repl_list(list);
}

// Creates a contiguous block of oif_lists and returns handle to the first one
hal_ret_t oif_list_create_block(oif_list_id_t *list, uint32_t size)
{
    return g_hal_state_pd->met_table()->create_repl_list_block(list, size);
}

// Takes an oiflis_handle and deletes it
hal_ret_t oif_list_delete(oif_list_id_t list)
{
    return g_hal_state_pd->met_table()->delete_repl_list(list);
}

// Takes an oiflis_handle and deletes a block starting from it
hal_ret_t oif_list_delete_block(oif_list_id_t list, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++) {
        oif_list_delete(list + i);
    }

    return HAL_RET_OK;
}

// Adds an oif to list
hal_ret_t oif_list_add_oif(oif_list_id_t list, oif_t *oif)
{
    hal_ret_t ret;
    p4_replication_data_t data = { 0 };
    if_t *pi_if = oif->intf;
    l2seg_t *pi_l2seg = oif->l2seg;

    HAL_ASSERT_RETURN(pi_if && pi_l2seg, HAL_RET_INVALID_ARG);

    ret = if_l2seg_get_multicast_rewrite_data(pi_if, pi_l2seg, &data);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    HAL_TRACE_DEBUG("Replication data: isTnl: {}; isQid: {}; rw_idx: {}; "
                    "tnl_idx: {}; lport : {} qtype : {} qid/vni : {}",
                    data.is_tunnel, data.is_qid, data.rewrite_index,
                    data.tunnel_rewrite_index, data.lport, data.qtype,
                    data.qid_or_vnid);

    return g_hal_state_pd->met_table()->add_replication(list, (void*)&data);
}

// Adds an rdma qp based oif to list
hal_ret_t oif_list_add_qp_oif(oif_list_id_t list, oif_t *oif)
{
    hal_ret_t ret;
    uint8_t is_tagged;
    uint16_t vlan_id;
    p4_replication_data_t data = { 0 };
    if_t *pi_if = oif->intf;
    l2seg_t *pi_l2seg = oif->l2seg;

    HAL_ASSERT_RETURN(pi_if && pi_l2seg, HAL_RET_INVALID_ARG);

    data.lport = if_get_lport_id(pi_if);

    HAL_ASSERT(hal::intf_get_if_type(pi_if) == intf::IF_TYPE_ENIC);
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

    HAL_TRACE_DEBUG("Replication data: isTnl: {}; isQid: {}; rw_idx: {}; "
                    "tnl_idx: {}; lport : {} qtype : {} qid/vni : {}",
                    data.is_tunnel, data.is_qid, data.rewrite_index,
                    data.tunnel_rewrite_index, data.lport, data.qtype,
                    data.qid_or_vnid);

    return g_hal_state_pd->met_table()->add_replication(list, (void*)&data);
}

// Removes an oif from list
// Removes an oif from list
hal_ret_t oif_list_remove_oif(oif_list_id_t list, oif_t *oif)
{
    p4_replication_data_t data = { 0 };

    // TODO: MET library expects the same data to be passed during
    //       removal as that passed during add. Otherwise it wont be
    //       able to find the replication.
    return g_hal_state_pd->met_table()->del_replication(list, (void*)&data);
}

// Check if an oif is present in the list
hal_ret_t oif_list_is_member(oif_list_id_t list, oif_t *oif) {
    return HAL_RET_OK;
}

// Get an array of all oifs in the list
hal_ret_t oif_list_get_num_oifs(oif_list_id_t list, uint32_t &num_oifs) {
    return HAL_RET_OK;
}

// Get an array of all oifs in the list
hal_ret_t oif_list_get_oif_array(oif_list_id_t list, uint32_t &num_oifs, oif_t *oifs) {
    return HAL_RET_OK;
}

// Adds a special node for ingress driven copy
hal_ret_t oif_list_set_honor_ingress(oif_list_id_t list)
{
    p4_replication_data_t data;
    memset(&data, 0, sizeof(data));
    data.repl_type = TM_REPL_TYPE_HONOR_INGRESS;
    return g_hal_state_pd->met_table()->add_replication(list, (void*)&data);
}

}    // namespace pd
}    // namespace hal


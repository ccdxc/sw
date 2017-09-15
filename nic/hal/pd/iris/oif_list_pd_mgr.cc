// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <base.h>
#include <p4pd.h>
#include <oif_list_pd_mgr.hpp>
#include <hal_state_pd.hpp>
#include <if_pd.hpp>
#include <if_pd_utils.hpp>

using namespace hal;

namespace hal {
namespace pd {

// Creates a new oif_list and returns handle
hal_ret_t oif_list_create(oif_list_id_t *list)
{
    return g_hal_state_pd->met_table()->create_repl_list(list);
}

// Takes an oiflis_handle and deletes it
hal_ret_t oif_list_delete(oif_list_id_t list)
{
    return g_hal_state_pd->met_table()->delete_repl_list(list);
}

// Adds an oif to list
hal_ret_t oif_list_add_oif(oif_list_id_t list, oif_t *oif)
{
    hal_ret_t ret;
    uint32_t encap_data;
    uint32_t rewrite_idx;
    uint32_t tnnl_rewrite_idx;
    p4_replication_data_t data;
    if_t *pi_if = find_if_by_id(oif->if_id);
    l2seg_t *pi_l2seg = find_l2seg_by_id(oif->l2_seg_id);

    HAL_ASSERT_RETURN(pi_if && pi_l2seg, HAL_RET_INVALID_ARG);

    ret = if_l2seg_get_encap_rewrite(pi_if, pi_l2seg, &encap_data, &rewrite_idx, &tnnl_rewrite_idx);

    if (ret != HAL_RET_OK){
        return ret;
    }

    data.lport = if_get_lport_id(pi_if);
    data.tunnel_rewrite_index = tnnl_rewrite_idx;
    data.qid_or_vnid = encap_data;
    data.rewrite_index = rewrite_idx;
    return g_hal_state_pd->met_table()->add_replication(list, (void*)&data);
}

// Removes an oif from list
hal_ret_t oif_list_remove_oif(oif_list_id_t list, oif_t *oif)
{
    p4_replication_data_t data;

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

}    // namespace pd
}    // namespace hal


// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.h"
#include "nic/gen/iris/include/p4pd.h"
#include "nic/hal/pd/iris/oif_list_pd_mgr.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/if_pd.hpp"
#include "nic/hal/pd/iris/if_pd_utils.hpp"
#include "nic/hal/pd/iris/tnnl_rw_pd.hpp"
#include "hal_state_pd.hpp"

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

hal_ret_t multicast_ip_to_mac(ip_addr_t *ip, mac_addr_t mac)
{
    uint64_t mac_val;

    if (ip->af == IP_AF_IPV4) {
        mac_val = ((0x01005ELL << 24) | (ip->addr.v4_addr & 0x7FFFFF));
    } else {
        uint64_t lower_ip_bits = ((1==ntohl(1)) ? (ip->addr.v6_addr.addr64[1])
                                                : (ip->addr.v6_addr.addr64[0]));
        mac_val = ((0x01005ELL << 24) | (lower_ip_bits & 0x7FFFFF));
    }

    mac[0] = (uint8_t)(((mac_val) >> 40) & 0xFF);
    mac[1] = (uint8_t)(((mac_val) >> 32) & 0xFF);
    mac[2] = (uint8_t)(((mac_val) >> 24) & 0xFF);
    mac[3] = (uint8_t)(((mac_val) >> 16) & 0xFF);
    mac[4] = (uint8_t)(((mac_val) >>  8) & 0xFF);
    mac[5] = (uint8_t)(((mac_val) >>  0) & 0xFF);
    return HAL_RET_OK;
}

// Adds an oif to list
hal_ret_t oif_list_add_oif(oif_list_id_t list, oif_t *oif)
{
    hal_ret_t ret;
    uint8_t is_tagged;
    uint16_t vlan_id;
    p4_replication_data_t data = { 0 };
    if_t *pi_if = oif->intf;
    l2seg_t *pi_l2seg = oif->l2seg;

    HAL_ASSERT_RETURN(pi_if && pi_l2seg, HAL_RET_INVALID_ARG);

    data.lport = if_get_lport_id(pi_if);

    switch (hal::intf_get_if_type(pi_if)) {
        case intf::IF_TYPE_ENIC: {
            hal::lif_t *lif = if_get_lif(pi_if);
            if (lif == NULL) {
                return HAL_RET_LIF_NOT_FOUND;
            }

            ret = if_l2seg_get_encap(pi_if, pi_l2seg, &is_tagged, &vlan_id);
            if (ret != HAL_RET_OK) {
                return ret;
            }

            data.is_qid = 1;
            data.qid_or_vnid = 0; // TODO refer to update_fwding_info()
            data.qtype = lif_get_qtype(lif, intf::LIF_QUEUE_PURPOSE_RX);
            data.rewrite_index = g_hal_state_pd->rwr_tbl_decap_vlan_idx();
            data.tunnel_rewrite_index = (is_tagged) ?
                                        (g_hal_state_pd->tnnl_rwr_tbl_encap_vlan_idx())
                                                    :
                                        (0);
            HAL_TRACE_DEBUG("Replication to Enic: lif_id: {}", lif->lif_id);
            break;
        }
        case intf::IF_TYPE_UPLINK:
        case intf::IF_TYPE_UPLINK_PC: {
            HAL_TRACE_DEBUG("Replication to Uplink: lport: {}", data.lport);
            switch (pi_l2seg->fabric_encap.type) {
                case types::encapType ::ENCAP_TYPE_NONE:
                case types::encapType ::ENCAP_TYPE_DOT1Q: {

                    ret = if_l2seg_get_encap(pi_if, pi_l2seg, &is_tagged,
                                             &vlan_id);
                    if (ret != HAL_RET_OK) {
                        return ret;
                    }

                    data.qid_or_vnid = vlan_id;
                    data.rewrite_index = g_hal_state_pd->rwr_tbl_decap_vlan_idx();
                    data.tunnel_rewrite_index = (is_tagged) ?
                                                (g_hal_state_pd->tnnl_rwr_tbl_encap_vlan_idx())
                                                            :
                                                (0);
                    break;
                }
                case types::encapType ::ENCAP_TYPE_VXLAN: {
                    uint32_t tnnl_rw_idx;
                    pd_tnnl_rw_entry_key_t tnnl_rw_key = {0};
                    l2seg_t *infra_seg = l2seg_get_infra_l2seg();
                    mac_addr_t *outer_mac = l2seg_get_rtr_mac(infra_seg);

                    ret = if_l2seg_get_encap(pi_if, infra_seg, &is_tagged,
                                             &vlan_id);
                    if (ret != HAL_RET_OK) {
                        return ret;
                    }

                    tnnl_rw_key.vlan_id = vlan_id;
                    tnnl_rw_key.vlan_valid = is_tagged;
                    tnnl_rw_key.tnnl_rw_act = TUNNEL_REWRITE_ENCAP_VXLAN_ID;
                    memcpy(&tnnl_rw_key.ip_sa, g_hal_state->oper_db()->mytep(),
                           sizeof(ip_addr_t));
                    memcpy(&tnnl_rw_key.ip_da, l2seg_get_gipo(pi_l2seg),
                           sizeof(ip_addr_t));
                    memcpy(&tnnl_rw_key.mac_sa, outer_mac, sizeof(mac_addr_t));
                    hal::pd::multicast_ip_to_mac(l2seg_get_gipo(pi_l2seg),
                                                 tnnl_rw_key.mac_da);

                    ret = tnnl_rw_entry_find_or_alloc(&tnnl_rw_key,
                                                      &tnnl_rw_idx);
                    if (ret != HAL_RET_OK) {
                        return ret;
                    }

                    data.qid_or_vnid = l2seg_get_fab_encap_val(pi_l2seg);
                    data.rewrite_index = g_hal_state_pd->rwr_tbl_decap_vlan_idx();
                    data.tunnel_rewrite_index = tnnl_rw_idx;
                    data.is_tunnel = 1;

                    HAL_TRACE_DEBUG("Replication to Tunnel: smac: {}; dmac: "
                                            "{}; vlan: {}; sip: {}; dip: {}; vnid: {}",
                                    macaddr2str(tnnl_rw_key.mac_sa),
                                    macaddr2str(tnnl_rw_key.mac_da),
                                    tnnl_rw_key.vlan_id,
                                    ipaddr2str(&tnnl_rw_key.ip_sa),
                                    ipaddr2str(&tnnl_rw_key.ip_da),
                                    data.qid_or_vnid);
                    break;
                }
                default:
                    HAL_ASSERT(0);
            }
            break;
        }
        default:
            HAL_ASSERT(0);
    }

    HAL_TRACE_DEBUG("Replication data: isTnl: {}; isQid: {}; rw_idx: {}; "
                    "tnl_idx: {}; lport : {} qtype : {} qid/vni : {}",
                    data.is_tunnel, data.is_qid, data.rewrite_index,
                    data.tunnel_rewrite_index, data.lport, data.qtype,
                    data.qid_or_vnid);

    return g_hal_state_pd->met_table()->add_replication(list, (void*)&data);
}

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

}    // namespace pd
}    // namespace hal


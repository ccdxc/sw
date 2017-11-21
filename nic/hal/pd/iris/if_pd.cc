#include "nic/include/hal_lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/hal/pd/iris/if_pd.hpp"
#include "nic/hal/pd/iris/uplinkif_pd.hpp"
#include "nic/hal/pd/iris/uplinkpc_pd.hpp"
#include "nic/hal/pd/iris/enicif_pd.hpp"
#include "nic/hal/pd/iris/cpuif_pd.hpp"
#include "nic/hal/pd/iris/tunnelif_pd.hpp"
#include "nic/hal/pd/iris/l2seg_uplink_pd.hpp"

namespace hal {
namespace pd {

// ----------------------------------------------------------------------------
// PD If Create
// ----------------------------------------------------------------------------
hal_ret_t
pd_if_create (pd_if_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;
    intf::IfType    if_type;

    HAL_TRACE_DEBUG("pd-if:{}: if create ", __FUNCTION__);


    if_type = hal::intf_get_if_type(args->intf);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            ret = pd_enicif_create(args);
            break;
        case intf::IF_TYPE_UPLINK:
            ret = pd_uplinkif_create(args);
            break;
        case intf::IF_TYPE_UPLINK_PC:
            ret = pd_uplinkpc_create(args);
            break;
        case intf::IF_TYPE_TUNNEL:
            ret = pd_tunnelif_create(args);
            break;
        case intf::IF_TYPE_CPU:
            ret = pd_cpuif_create(args);
            break;
        default:
            HAL_ASSERT(0);
    }
    // Branch out for different interface types
    return ret;
}

// ----------------------------------------------------------------------------
// PD If Update
// ----------------------------------------------------------------------------
hal_ret_t
pd_if_update (pd_if_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;
    intf::IfType    if_type;

    HAL_TRACE_DEBUG("pd-if:{}: if update", __FUNCTION__);

    if_type = hal::intf_get_if_type(args->intf);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            ret = pd_enicif_update(args);
            break;
        case intf::IF_TYPE_UPLINK:
            ret = pd_uplinkif_update(args);
            break;
        case intf::IF_TYPE_UPLINK_PC:
            ret = pd_uplinkpc_update(args);
            break;
        case intf::IF_TYPE_TUNNEL:
            ret = pd_tunnelif_update(args);
            break;
        case intf::IF_TYPE_CPU:
            ret = pd_cpuif_update(args);
            break;
        default:
            HAL_ASSERT(0);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// PD If Delete
// ----------------------------------------------------------------------------
hal_ret_t
pd_if_delete (pd_if_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;
    intf::IfType    if_type;

    HAL_TRACE_DEBUG("pd-if:{}: if delete", __FUNCTION__);

    if_type = hal::intf_get_if_type(args->intf);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            ret = pd_enicif_delete(args);
            break;
        case intf::IF_TYPE_UPLINK:
            ret = pd_uplinkif_delete(args);
            break;
        case intf::IF_TYPE_UPLINK_PC:
            ret = pd_uplinkpc_delete(args);
            break;
        case intf::IF_TYPE_TUNNEL:
            ret = pd_tunnelif_delete(args);
            break;
        case intf::IF_TYPE_CPU:
            ret = pd_cpuif_delete(args);
            break;
        default:
            HAL_ASSERT(0);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Lif params update. For enics
// ----------------------------------------------------------------------------
hal_ret_t 
pd_if_lif_update(pd_if_lif_upd_args_t *args)
{
    HAL_TRACE_DEBUG("pd-if:{}: if lif update", __FUNCTION__);
    return pd_enicif_lif_update(args);
}



// ----------------------------------------------------------------------------
// PD If mem free
// ----------------------------------------------------------------------------
hal_ret_t
pd_if_mem_free (pd_if_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;
    intf::IfType    if_type;

    HAL_TRACE_DEBUG("pd-if:{}: if mem_free", __FUNCTION__);

    if_type = hal::intf_get_if_type(args->intf);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            ret = pd_enicif_mem_free(args);
            break;
        case intf::IF_TYPE_UPLINK:
            ret = pd_uplinkif_mem_free(args);
            break;
        case intf::IF_TYPE_UPLINK_PC:
            ret = pd_uplinkpc_mem_free(args);
            break;
        case intf::IF_TYPE_TUNNEL:
            ret = pd_tunnelif_mem_free(args);
            break;
        case intf::IF_TYPE_CPU:
            ret = pd_cpuif_mem_free(args);
            break;
        default:
            HAL_ASSERT(0);
    }

    return ret;
}
hal_ret_t
pd_if_make_clone (if_t *hal_if, if_t *clone)
{
    hal_ret_t       ret = HAL_RET_OK;
    intf::IfType    if_type;

    HAL_TRACE_DEBUG("pd-if:{}: if clone", __FUNCTION__);

    if_type = hal::intf_get_if_type(hal_if);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            ret = pd_enicif_make_clone(hal_if, clone);
            break;
        case intf::IF_TYPE_UPLINK:
            ret = pd_uplinkif_make_clone(hal_if, clone);
            break;
        case intf::IF_TYPE_UPLINK_PC:
            ret = pd_uplinkpc_make_clone(hal_if, clone);
            break;
        case intf::IF_TYPE_TUNNEL:
            ret = pd_tunnelif_make_clone(hal_if, clone);
            break;
        case intf::IF_TYPE_CPU:
            ret = pd_cpuif_make_clone(hal_if, clone);
            break;
        default:
            HAL_ASSERT(0);
    }

    return ret;
}
//-----------------------------------------------------------------------------
// Nwsec profile update 
//      - Triggered from vrf update
//-----------------------------------------------------------------------------
hal_ret_t
pd_if_nwsec_update(pd_if_nwsec_upd_args_t *args)
{
    hal_ret_t                   ret = HAL_RET_OK;
    intf::IfType                if_type;
    void                        *pd_if = args->intf->pd_if;
    pd_l2seg_uplink_args_t      uplink_args = { 0 };

    HAL_TRACE_DEBUG("pd-if:{}:", __FUNCTION__);

    if_type = hal::intf_get_if_type(args->intf);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            ret = pd_enicif_upd_inp_prop_mac_vlan_tbl((pd_enicif_t *)pd_if,
                                                      args->nwsec_prof);
            break;
        case intf::IF_TYPE_UPLINK:
        case intf::IF_TYPE_UPLINK_PC:
            uplink_args.l2seg = args->l2seg;
            uplink_args.intf = args->intf;
            ret = l2seg_uplink_upd_input_properties_tbl(&uplink_args, 
                                                        args->nwsec_prof);
            break;
        default:
            HAL_ASSERT(0);
    }

    return ret;
}



// ----------------------------------------------------------------------------
// Returns the encap data and rewrite idx used for l2seg on an if. This is to be called from pd side
// Assumption: Ingress & Egress are same.
// ----------------------------------------------------------------------------
hal_ret_t
if_l2seg_get_encap_rewrite(if_t *pi_if, l2seg_t *pi_l2seg, uint32_t *encap_data,
                           uint32_t *rewrite_idx, uint32_t *tnnl_rewrite_idx)
{
    hal_ret_t ret = HAL_RET_OK;

    HAL_ASSERT_RETURN(pi_if && pi_l2seg && encap_data, HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN(rewrite_idx && tnnl_rewrite_idx, HAL_RET_INVALID_ARG);

    switch(hal::intf_get_if_type(pi_if)) {
        case intf::IF_TYPE_ENIC:
        case intf::IF_TYPE_UPLINK:
        case intf::IF_TYPE_UPLINK_PC:
        {
                uint8_t is_tagged;
                uint16_t vlan_id;

                ret = if_l2seg_get_encap(pi_if, pi_l2seg, &is_tagged, &vlan_id);

                if (ret != HAL_RET_OK) {
                    break;
                }

                (*encap_data) = vlan_id;

                if (is_tagged) {
                    *tnnl_rewrite_idx = TUNNEL_REWRITE_ENCAP_VLAN_ID;
                } else {
                    *tnnl_rewrite_idx = TUNNEL_REWRITE_NOP_ID;
                }

                *rewrite_idx = REWRITE_NOP_ID;
                break;
        }
        case intf::IF_TYPE_TUNNEL:
            // TODO: Handle for Tunnel case
            break;
        default:
            HAL_ASSERT(0);
    }

    return ret;
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

hal_ret_t if_l2seg_get_multicast_rewrite_data(if_t *pi_if, l2seg_t *pi_l2seg,
                                              p4_replication_data_t *data)
{
    hal_ret_t ret;
    uint8_t is_tagged;
    uint16_t vlan_id;

    HAL_ASSERT_RETURN(pi_if && pi_l2seg && data, HAL_RET_INVALID_ARG);

    data->lport = if_get_lport_id(pi_if);
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

            data->is_qid = 1;
            data->qid_or_vnid = 0; // TODO refer to update_fwding_info()
            data->qtype = lif_get_qtype(lif, intf::LIF_QUEUE_PURPOSE_RX);

            switch (pi_if->enic_type) {
                case intf::IF_ENIC_TYPE_CLASSIC:
                    data->rewrite_index = (is_tagged) ?
                                          (0):
                                          (g_hal_state_pd->rwr_tbl_decap_vlan_idx());
                    data->tunnel_rewrite_index = 0;
                    break;
                default:
                    data->rewrite_index = g_hal_state_pd->rwr_tbl_decap_vlan_idx();
                    data->tunnel_rewrite_index = (is_tagged) ?
                                                 (g_hal_state_pd->tnnl_rwr_tbl_encap_vlan_idx()):
                                                 (0);
                    break;
            }
            HAL_TRACE_DEBUG("Replication to Enic: lif_id: {}", lif->lif_id);
            break;
        }
        case intf::IF_TYPE_UPLINK:
        case intf::IF_TYPE_UPLINK_PC: {
            HAL_TRACE_DEBUG("Replication to Uplink: lport: {}", data->lport);
            switch (pi_l2seg->wire_encap.type) {
                case types::encapType ::ENCAP_TYPE_NONE:
                case types::encapType ::ENCAP_TYPE_DOT1Q: {

                    ret = if_l2seg_get_encap(pi_if, pi_l2seg, &is_tagged,
                                             &vlan_id);
                    if (ret != HAL_RET_OK) {
                        return ret;
                    }

                    data->qid_or_vnid = vlan_id;
                    data->rewrite_index = g_hal_state_pd->rwr_tbl_decap_vlan_idx();
                    data->tunnel_rewrite_index = (is_tagged) ?
                                                 (g_hal_state_pd->tnnl_rwr_tbl_encap_vlan_idx()):
                                                 (0);
                    break;
                }
                case types::encapType ::ENCAP_TYPE_VXLAN: {
                    uint32_t tnnl_rw_idx;
                    pd_tnnl_rw_entry_key_t tnnl_rw_key = {0};
                    l2seg_t *infra_seg = l2seg_get_infra_l2seg();
                    mac_addr_t *outer_mac = l2seg_get_rtr_mac(infra_seg);

                    ret = if_l2seg_get_encap(pi_if, infra_seg, &is_tagged, &vlan_id);
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

                    data->qid_or_vnid = l2seg_get_wire_encap_val(pi_l2seg);
                    data->rewrite_index = g_hal_state_pd->rwr_tbl_decap_vlan_idx();
                    data->tunnel_rewrite_index = tnnl_rw_idx;
                    data->is_tunnel = 1;

                    HAL_TRACE_DEBUG("Replication to Tunnel: smac: {}; dmac: "
                                            "{}; vlan: {}; sip: {}; dip: {}; vnid: {}",
                                    macaddr2str(tnnl_rw_key.mac_sa),
                                    macaddr2str(tnnl_rw_key.mac_da),
                                    tnnl_rw_key.vlan_id,
                                    ipaddr2str(&tnnl_rw_key.ip_sa),
                                    ipaddr2str(&tnnl_rw_key.ip_da),
                                    data->qid_or_vnid);
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

    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal

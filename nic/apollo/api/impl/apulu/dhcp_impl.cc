//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of DHCP objects
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/dhcp.hpp"
#include "nic/apollo/api/impl/apulu/dhcp_impl.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"
#include "nic/apollo/api/impl/apulu/nacl_data.h"

namespace api {
namespace impl {

/// \defgroup PDS_DHCP_IMPL - DHCP datapath implementation
/// \ingroup PDS_DHCP
/// \@{

dhcp_relay_impl *
dhcp_relay_impl::factory(pds_dhcp_relay_spec_t *spec) {
    dhcp_relay_impl *impl;

    impl = dhcp_impl_db()->alloc_relay();
    if (unlikely(impl == NULL)) {
        return NULL;
    }
    new (impl) dhcp_relay_impl();
    return impl;
}

void
dhcp_relay_impl::soft_delete(dhcp_relay_impl *impl) {
    impl->~dhcp_relay_impl();
    dhcp_impl_db()->free(impl);
}

void
dhcp_relay_impl::destroy(dhcp_relay_impl *impl) {
    dhcp_relay_impl::soft_delete(impl);
}

impl_base *
dhcp_relay_impl::clone(void) {
    dhcp_relay_impl *cloned_impl;

    cloned_impl = dhcp_impl_db()->alloc_relay();
    new (cloned_impl) dhcp_relay_impl();
    // deep copy is not needed as we don't store pointers
    *cloned_impl = *this;
    return cloned_impl;
}

sdk_ret_t
dhcp_relay_impl::free(dhcp_relay_impl *impl) {
    destroy(impl);
    return SDK_RET_OK;
}

dhcp_relay_impl *
dhcp_relay_impl::build(pds_obj_key_t *key, dhcp_relay *relay) {
    dhcp_relay_impl *impl;

    impl = dhcp_impl_db()->alloc_relay();
    if (unlikely(impl == NULL)) {
        return NULL;
    }
    new (impl) dhcp_relay_impl();
    return impl;
}

#define nacl_redirect_action    action_u.nacl_nacl_redirect
#define nacl_redirect_to_arm_action     action_u.nacl_nacl_redirect_to_arm
sdk_ret_t
dhcp_relay_impl::activate_hw(api_base *api_obj, api_base *orig_obj,
                             pds_epoch_t epoch, api_op_t api_op,
                             api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    p4pd_error_t p4pd_ret;
    nacl_swkey_t key = { 0 };
    nacl_actiondata_t data = { 0 };
    nacl_swkey_mask_t mask = { 0 };
    pds_dhcp_relay_spec_t *spec;

    switch (api_op) {
    case API_OP_CREATE:
    case API_OP_UPDATE:
        // install the NACL
        spec = &obj_ctxt->api_params->dhcp_relay_spec;
        key.control_metadata_rx_packet = 1;
        key.key_metadata_ktype = KEY_TYPE_IPV4;
        key.arm_to_p4i_nexthop_valid = 0;
        key.control_metadata_lif_type = P4_LIF_TYPE_UPLINK;
        key.control_metadata_flow_miss = 1;
        key.control_metadata_tunneled_packet = 0;
        key.key_metadata_sport = 67;
        key.key_metadata_proto = 17;
        // destination IP is the local DHCP relay agent's IP address
        IPADDR_TO_P4_IPADDR(key.key_metadata_dst, &spec->agent_ip, IP_AF_IPV4);
        // source IP is the DHCP server's IP address
        IPADDR_TO_P4_IPADDR(key.key_metadata_src, &spec->server_ip, IP_AF_IPV4);
        // dhcpd seems to be using dest port 67
        key.key_metadata_dport = 67;

        mask.control_metadata_rx_packet_mask = ~0;
        mask.key_metadata_ktype_mask = ~0;
        mask.arm_to_p4i_nexthop_valid_mask = ~0;
        mask.control_metadata_lif_type_mask = ~0;
        mask.control_metadata_flow_miss_mask = ~0;
        mask.control_metadata_tunneled_packet_mask = ~0;
        mask.key_metadata_sport_mask = ~0;
        mask.key_metadata_proto_mask = ~0;
        for (uint8_t i = 0; i < sizeof(mask.key_metadata_dst_mask); i++) {
            mask.key_metadata_dst_mask[i] = ~0;
        }
        for (uint8_t i = 0; i < sizeof(mask.key_metadata_src_mask); i++) {
            mask.key_metadata_src_mask[i] = ~0;
        }
        mask.key_metadata_dport_mask = ~0;
        data.action_id = NACL_NACL_REDIRECT_TO_ARM_ID;
        data.nacl_redirect_to_arm_action.nexthop_type = NEXTHOP_TYPE_NEXTHOP;
        data.nacl_redirect_to_arm_action.nexthop_id =
             lif_impl_db()->find(sdk::platform::LIF_TYPE_MNIC_CPU)->nh_idx();
        //data.nacl_redirect_to_arm_action.copp_policer_id = idx;
        data.nacl_redirect_to_arm_action.data = NACL_DATA_ID_FLOW_MISS_DHCP;
        p4pd_ret = p4pd_entry_install(P4TBL_ID_NACL,
                                      PDS_IMPL_RSVD_DHCP_RELAY_NACL_IDX1, &key,
                                      &mask, &data);
        if (p4pd_ret != P4PD_SUCCESS) {
            PDS_TRACE_ERR("Failed to program DHCP relay NACL redirect entry "
                          "at %u", PDS_IMPL_RSVD_DHCP_RELAY_NACL_IDX1);
            return sdk::SDK_RET_HW_PROGRAM_ERR;
        }

        // cover the regular case where dst port is 68
        key.key_metadata_dport = 68;
        p4pd_ret = p4pd_entry_install(P4TBL_ID_NACL,
                                      PDS_IMPL_RSVD_DHCP_RELAY_NACL_IDX2, &key,
                                      &mask, &data);
        if (p4pd_ret != P4PD_SUCCESS) {
            PDS_TRACE_ERR("Failed to program DHCP relay NACL redirect entry "
                          "at %u", PDS_IMPL_RSVD_DHCP_RELAY_NACL_IDX2);
            return sdk::SDK_RET_HW_PROGRAM_ERR;
        }
        break;

    case API_OP_DELETE:
        // clear the DHCP relay related NACLs
        p4pd_ret = p4pd_entry_install(P4TBL_ID_NACL,
                                      PDS_IMPL_RSVD_DHCP_RELAY_NACL_IDX1, &key,
                                      &mask, &data);
        if (p4pd_ret != P4PD_SUCCESS) {
            PDS_TRACE_ERR("Failed to clear DHCP relay NACL redirect entry, "
                          "idx %u", PDS_IMPL_RSVD_DHCP_RELAY_NACL_IDX1);
            return sdk::SDK_RET_HW_PROGRAM_ERR;
        }
        p4pd_ret = p4pd_entry_install(P4TBL_ID_NACL,
                                      PDS_IMPL_RSVD_DHCP_RELAY_NACL_IDX2, &key,
                                      &mask, &data);
        if (p4pd_ret != P4PD_SUCCESS) {
            PDS_TRACE_ERR("Failed to clear DHCP relay NACL redirect entry, "
                          "idx %u", PDS_IMPL_RSVD_DHCP_RELAY_NACL_IDX2);
            return sdk::SDK_RET_HW_PROGRAM_ERR;
        }
        break;

    default:
        ret = sdk::SDK_RET_INVALID_OP;
        break;
    }
    return ret;
}

sdk_ret_t
dhcp_relay_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    return SDK_RET_INVALID_OP;
}

/// \@}    // end of PDS_DHCP_IMPL

}    // namespace impl
}    // namespace api

// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
#include "nic/metaswitch/stubs/mgmt/pds_ms_interface.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/mgmt/pdsa_internal_utils_gen.hpp"
#include "gen/proto/internal.pb.h"
#include "nic/metaswitch/stubs/common/pds_ms_ifindex.hpp"

namespace pds_ms {

static void
populate_lim_l3_intf_cfg_spec ( pds::LimInterfaceCfgSpec& req, uint32_t ifindex)
{
    req.set_entityindex (PDSA_LIM_ENT_INDEX);
    req.set_ifindex (ifindex);
    req.set_ifenable (AMB_TRUE);
    req.set_ipv4enabled (AMB_TRISTATE_TRUE);
    req.set_ipv4fwding (AMB_TRISTATE_TRUE);
    req.set_ipv6enabled (AMB_TRISTATE_TRUE);
    req.set_ipv6fwding (AMB_TRISTATE_TRUE);
    req.set_fwdingmode (AMB_LIM_FWD_MODE_L3);
    req.set_vrfnamelen (0);
}

static void
populate_lim_addr_spec (pds_if_spec_t                *if_spec,
                        pds::LimInterfaceAddrSpec&   req,
                        uint32_t                     if_index)
{
    auto ifipaddr = req.mutable_ifipaddr();

    req.set_entityindex (PDSA_LIM_ENT_INDEX);
    req.set_ifindex (if_index);
    req.set_prefixlen (if_spec->l3_if_info.ip_prefix.len);

    if (if_spec->l3_if_info.ip_prefix.addr.af == IP_AF_IPV4) {
        ifipaddr->set_af(types::IP_AF_INET);
        ifipaddr->set_v4addr(if_spec->l3_if_info.ip_prefix.addr.addr.v4_addr);
    } else {
        ifipaddr->set_af(types::IP_AF_INET6);
        ifipaddr->set_v6addr(if_spec->l3_if_info.ip_prefix.addr.addr.v6_addr.addr8, 
                             IP6_ADDR8_LEN);
    }
}

static void
process_interface_update (pds_if_spec_t *if_spec,
                          NBB_LONG      row_status)
{
    uint32_t ms_ifindex = 0;

    PDSA_GET_SHARED_START();
    NBB_TRC_ENTRY ("process_interface_update");

    // Start CTM transaction
    pdsa_ctm_send_transaction_start (PDSA_CTM_GRPC_CORRELATOR);

    // Get PDS to MS IfIndex
    ms_ifindex = pds_to_ms_ifindex(if_spec->key.id, IF_TYPE_ETH);
    SDK_TRACE_INFO ("L3 Intf Cfg:: IfIndex: PDS[0x%X] to MS[0x%X]]\n", if_spec->key.id, ms_ifindex);

    // Create L3 interfaces
    pds::LimInterfaceCfgSpec lim_if_spec;
    populate_lim_l3_intf_cfg_spec (lim_if_spec, ms_ifindex);
    pdsa_set_amb_lim_if_cfg (lim_if_spec, row_status, PDSA_CTM_GRPC_CORRELATOR);

    // Configure IP Address
    pds::LimInterfaceAddrSpec lim_addr_spec;
    populate_lim_addr_spec (if_spec, lim_addr_spec, ms_ifindex);
    pdsa_set_amb_lim_l3_if_addr (lim_addr_spec, row_status, PDSA_CTM_GRPC_CORRELATOR);

    // End CTM transaction
    pdsa_ctm_send_transaction_end (PDSA_CTM_GRPC_CORRELATOR);

    NBB_TRC_EXIT();
    PDSA_GET_SHARED_END();

    // blocking on response from MS
    pds_ms::mgmt_state_t::ms_response_wait();

    // TODO: read return status from ms_response and send to caller

    return;
}

sdk_ret_t
interface_create (pds_if_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    process_interface_update (spec, AMB_ROW_ACTIVE);

    // TODO: Get correct return code from CTM callback
    return SDK_RET_OK;
}

sdk_ret_t
interface_delete (pds_if_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    process_interface_update (spec, AMB_ROW_DESTROY);

    // TODO: Get correct return code from CTM callback
    return SDK_RET_OK;
}

sdk_ret_t
interface_update (pds_if_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    // TODO
    return SDK_RET_OK;
}

};    // namespace pds_ms

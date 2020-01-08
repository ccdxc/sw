// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
#include "nic/metaswitch/stubs/mgmt/pds_ms_interface.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/mgmt/pds_ms_internal_utils_gen.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/mgmt/pds_ms_cp_interface_utils_gen.hpp"
#include "gen/proto/internal.pb.h"
#include "nic/metaswitch/stubs/common/pds_ms_ifindex.hpp"

namespace pds_ms {

static void
populate_lim_l3_intf_cfg_spec ( pds::LimInterfaceCfgSpec& req, uint32_t ifindex)
{
    req.set_entityindex (PDS_MS_LIM_ENT_INDEX);
    req.set_ifindex (ifindex);
    req.set_ifenable (AMB_TRUE);
    req.set_ipv4enabled (AMB_TRISTATE_TRUE);
    req.set_ipv4fwding (AMB_TRISTATE_TRUE);
    req.set_ipv6enabled (AMB_TRISTATE_TRUE);
    req.set_ipv6fwding (AMB_TRISTATE_TRUE);
    req.set_fwdingmode (AMB_LIM_FWD_MODE_L3);
}

void
populate_lim_addr_spec (ip_prefix_t                 *ip_prefix,
                        pds::CPInterfaceAddrSpec&   req,
                        uint32_t                    if_type,
                        uint32_t                    if_id)
{
    auto ifipaddr = req.mutable_ipaddr();

    req.set_iftype ((pds::CPIntfType)if_type);
    req.set_ifid (if_id);
    req.set_prefixlen (ip_prefix->len);

    if (ip_prefix->addr.af == IP_AF_IPV4) {
        ifipaddr->set_af(types::IP_AF_INET);
        ifipaddr->set_v4addr(ip_prefix->addr.addr.v4_addr);
    } else {
        ifipaddr->set_af(types::IP_AF_INET6);
        ifipaddr->set_v6addr(ip_prefix->addr.addr.v6_addr.addr8, 
                             IP6_ADDR8_LEN);
    }
}

static types::ApiStatus
process_interface_update (pds_if_spec_t *if_spec,
                          NBB_LONG      row_status)
{
    uint32_t ms_ifindex = 0;

    PDS_MS_START_TXN(PDS_MS_CTM_GRPC_CORRELATOR);

    // Get PDS to MS IfIndex
    ms_ifindex = pds_to_ms_ifindex(if_spec->key.id, IF_TYPE_ETH);
    SDK_TRACE_INFO ("L3 Intf Cfg:: IfIndex: PDS[0x%X] to MS[0x%X]]\n", if_spec->key.id, ms_ifindex);

    // Create L3 interfaces
    pds::LimInterfaceCfgSpec lim_if_spec;
    populate_lim_l3_intf_cfg_spec (lim_if_spec, ms_ifindex);
    pds_ms_set_amb_lim_if_cfg (lim_if_spec, row_status, PDS_MS_CTM_GRPC_CORRELATOR);

    // Configure IP Address
    pds::CPInterfaceAddrSpec lim_addr_spec;
    populate_lim_addr_spec (&if_spec->l3_if_info.ip_prefix, lim_addr_spec, 
                            pds::CP_IF_TYPE_ETH, if_spec->key.id);
    pds_ms_set_amb_lim_l3_if_addr (lim_addr_spec, row_status, PDS_MS_CTM_GRPC_CORRELATOR);

    PDS_MS_END_TXN(PDS_MS_CTM_GRPC_CORRELATOR);

    // blocking on response from MS
    return pds_ms::mgmt_state_t::ms_response_wait();
}

sdk_ret_t
interface_create (pds_if_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    types::ApiStatus ret_status;
    ret_status = process_interface_update (spec, AMB_ROW_ACTIVE);

    if (ret_status != types::ApiStatus::API_STATUS_OK) {
        SDK_TRACE_ERR ("Failed to process interface 0x%X create (error=%d)\n", 
                        spec->key.id, ret_status);
        return pds_ms_api_to_sdk_ret (ret_status);
    }

    SDK_TRACE_DEBUG ("interface 0x%X create is successfully processed\n", 
                      spec->key.id);
    return SDK_RET_OK;
}

sdk_ret_t
interface_delete (pds_if_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    types::ApiStatus ret_status;
    ret_status = process_interface_update (spec, AMB_ROW_DESTROY);

    if (ret_status != types::ApiStatus::API_STATUS_OK) {
        SDK_TRACE_ERR ("Failed to process interface 0x%X delete (error=%d)\n", 
                        spec->key.id, ret_status);
        return pds_ms_api_to_sdk_ret (ret_status);
    }

    SDK_TRACE_DEBUG ("interface 0x%X delete is successfully processed\n", 
                      spec->key.id);
    return SDK_RET_OK;
}

sdk_ret_t
interface_update (pds_if_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    // TODO
    return SDK_RET_OK;
}

};    // namespace pds_ms

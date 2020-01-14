// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
#include "nic/metaswitch/stubs/mgmt/pds_ms_interface.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/mgmt/pds_ms_internal_utils_gen.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/mgmt/pds_ms_cp_interface_utils_gen.hpp"
#include "gen/proto/internal.pb.h"
#include "nic/metaswitch/stubs/common/pds_ms_ifindex.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_defs.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"

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
                          ms_ifindex_t ms_ifindex,
                          NBB_LONG      row_status)
{
    PDS_MS_START_TXN(PDS_MS_CTM_GRPC_CORRELATOR);

    // Create L3 interfaces
    pds::LimInterfaceCfgSpec lim_if_spec;
    populate_lim_l3_intf_cfg_spec (lim_if_spec, ms_ifindex);
    pds_ms_set_amb_lim_if_cfg (lim_if_spec, row_status, PDS_MS_CTM_GRPC_CORRELATOR);

    // Configure IP Address
    pds::CPInterfaceAddrSpec lim_addr_spec;
    populate_lim_addr_spec (&if_spec->l3_if_info.ip_prefix, lim_addr_spec, 
                            pds::CP_IF_TYPE_ETH, ms_ifindex);
    pds_ms_set_amb_lim_l3_if_addr (lim_addr_spec, row_status,
                                   PDS_MS_CTM_GRPC_CORRELATOR);

    PDS_MS_END_TXN(PDS_MS_CTM_GRPC_CORRELATOR);

    // blocking on response from MS
    return pds_ms::mgmt_state_t::ms_response_wait();
}

sdk_ret_t
interface_create (pds_if_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    types::ApiStatus ret_status;
    SDK_ASSERT(spec->type == PDS_IF_TYPE_L3);

    // Get PDS to MS IfIndex
    auto eth_ifindex = spec->l3_if_info.eth_ifindex;

    auto ms_ifindex = pds_to_ms_ifindex(eth_ifindex, IF_TYPE_ETH);
    SDK_TRACE_INFO ("L3 Intf Create:: UUID %s Eth[0x%X] to MS[0x%X]]",
                    spec->key.str(), eth_ifindex, ms_ifindex);

    // TODO Cache L3 Intf UUID to MS IfIndex

    ret_status = process_interface_update (spec, ms_ifindex, AMB_ROW_ACTIVE);

    if (ret_status != types::ApiStatus::API_STATUS_OK) {
        SDK_TRACE_ERR ("Failed to process L3 interface %s create for "
                       "MSIfIndex 0x%X err %d",
                        spec->key.str(), ms_ifindex, ret_status);
        return pds_ms_api_to_sdk_ret (ret_status);
    }

    SDK_TRACE_DEBUG ("L3 intf create for Eth 0x%X MSIfIndex 0x%X"
                     " successfully processed",
                      eth_ifindex, ms_ifindex);
    return SDK_RET_OK;
}

sdk_ret_t
interface_delete (pds_if_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    types::ApiStatus ret_status;
    // TODO - Fill MS IfIndex from UUID cache
    ms_ifindex_t ms_ifindex = pdsobjkey2msidx(spec->key);

    ret_status = process_interface_update (spec, ms_ifindex, AMB_ROW_DESTROY);

    if (ret_status != types::ApiStatus::API_STATUS_OK) {
        SDK_TRACE_ERR ("Failed to process interface UUID %s MS-Interface 0x%X "
                       "delete err %d", 
                        spec->key.str(), ms_ifindex, ret_status);
        return pds_ms_api_to_sdk_ret (ret_status);
    }

    SDK_TRACE_DEBUG ("L3 Interface delete for MSIfIndex 0x%X successfully processed",
                      ms_ifindex);
    return SDK_RET_OK;
}

sdk_ret_t
interface_update (pds_if_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    // TODO
    SDK_TRACE_INFO ("L3 Intf Update:: UUID %s", spec->key.str());

    return SDK_RET_OK;
}

};    // namespace pds_ms

// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
#include "nic/metaswitch/stubs/mgmt/pds_ms_interface.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/mgmt/pds_ms_internal_utils_gen.hpp"
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
                        pds::LimInterfaceAddrSpec&   req,
                        uint32_t                    if_type,
                        uint32_t                    if_id)
{
    auto ifipaddr = req.mutable_ipaddr();

    req.set_iftype ((pds::LimIntfType)if_type);
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
    pds::LimInterfaceAddrSpec lim_addr_spec;
    PDS_MS_START_TXN(PDS_MS_CTM_GRPC_CORRELATOR);

    if (if_spec->type == PDS_IF_TYPE_L3) {
        // Create L3 interfaces
        pds::LimInterfaceCfgSpec lim_if_spec;
        populate_lim_l3_intf_cfg_spec (lim_if_spec, ms_ifindex);
        pds_ms_set_amb_lim_if_cfg (lim_if_spec, row_status, 
                                   PDS_MS_CTM_GRPC_CORRELATOR);
        populate_lim_addr_spec (&if_spec->l3_if_info.ip_prefix, lim_addr_spec, 
                                pds::LIM_IF_TYPE_ETH, 
                                if_spec->l3_if_info.eth_ifindex);
    } else if (if_spec->type == PDS_IF_TYPE_LOOPBACK){
        // Otherwise, its always singleton loopback
        pds::LimInterfaceSpec lim_swif_spec;
        lim_swif_spec.set_ifid(LOOPBACK_IF_ID);
        lim_swif_spec.set_iftype(pds::LIM_IF_TYPE_LOOPBACK);
        pds_ms_set_amb_lim_software_if (lim_swif_spec, row_status,
                                        PDS_MS_CTM_GRPC_CORRELATOR);
        populate_lim_addr_spec (&if_spec->loopback_if_info.ip_prefix, 
                                lim_addr_spec, pds::LIM_IF_TYPE_LOOPBACK,
                                LOOPBACK_IF_ID);
    }

    // Configure IP Address
    pds_ms_set_amb_lim_l3_if_addr (lim_addr_spec, row_status,
                                   PDS_MS_CTM_GRPC_CORRELATOR);

    PDS_MS_END_TXN(PDS_MS_CTM_GRPC_CORRELATOR);

    // blocking on response from MS
    return pds_ms::mgmt_state_t::ms_response_wait();
}

void
interface_uuid_alloc(const pds_obj_key_t& key, uint32_t &ms_ifindex)
{
    auto mgmt_ctxt = mgmt_state_t::thread_context();
    auto uuid_obj = mgmt_ctxt.state()->lookup_uuid(key);
    if (uuid_obj != nullptr) {
        throw Error(std::string("Interface Create for existing UUID")
                    .append(key.str()).append(" containing ")
                    .append(uuid_obj->str()), SDK_RET_ENTRY_EXISTS);
    }
    interface_uuid_obj_uptr_t interface_uuid_obj
                              (new interface_uuid_obj_t(key, ms_ifindex));
    mgmt_ctxt.state()->set_pending_uuid_create(key,
                                               std::move(interface_uuid_obj));
    return;
}

uint32_t
interface_uuid_fetch(const pds_obj_key_t& key, bool del_op)
{
    auto mgmt_ctxt = mgmt_state_t::thread_context();
    auto uuid_obj = mgmt_ctxt.state()->lookup_uuid(key);
    if (uuid_obj == nullptr) {
        throw Error(std::string("Unknown UUID in Interface Request ")
                    .append(key.str()), SDK_RET_ENTRY_NOT_FOUND);
    }
    if (uuid_obj->obj_type() != uuid_obj_type_t::INTERFACE) {
        throw Error(std::string("Wrong UUID ").append(key.str())
                    .append(" containing ").append(uuid_obj->str())
                    .append(" in Interface request"), SDK_RET_INVALID_ARG);
    }
    auto interface_uuid_obj = (interface_uuid_obj_t*) uuid_obj;
    SDK_TRACE_VERBOSE("Fetched Interface UUID %s MSIfindex 0x%X",
                      key.str(), interface_uuid_obj->ms_id().ms_ifindex);
    if (del_op) {
        mgmt_ctxt.state()->set_pending_uuid_delete(key);
    }
    return interface_uuid_obj->ms_id().ms_ifindex;
}

sdk_ret_t
interface_create (pds_if_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    types::ApiStatus ret_status;
    uint32_t ms_ifindex = 0;

    SDK_ASSERT((spec->type == PDS_IF_TYPE_L3 ||
                spec->type == PDS_IF_TYPE_LOOPBACK));

    try {
        // Get PDS to MS IfIndex
        if (spec->type == PDS_IF_TYPE_L3) {
           auto eth_ifindex = spec->l3_if_info.eth_ifindex;

            ms_ifindex = pds_to_ms_ifindex(eth_ifindex, IF_TYPE_ETH);
            SDK_TRACE_INFO ("L3 Intf Create:: UUID %s Eth[0x%X] to MS[0x%X]]",
                            spec->key.str(), eth_ifindex, ms_ifindex);
        } else if (spec->type == PDS_IF_TYPE_LOOPBACK) {
            ms_ifindex = pds_to_ms_ifindex(LOOPBACK_IF_ID, IF_TYPE_LOOPBACK);
            SDK_TRACE_INFO ("Loopback Intf Create:: UUID %s to MS[0x%X]]",
                            spec->key.str(),  ms_ifindex);
        }
        // Cache Intf UUID to MS IfIndex
        interface_uuid_alloc(spec->key, ms_ifindex);
    
        ret_status = process_interface_update (spec, ms_ifindex, AMB_ROW_ACTIVE);
        if (ret_status != types::ApiStatus::API_STATUS_OK) {
            SDK_TRACE_ERR ("Failed to process interface %s create for "
                           "MSIfIndex 0x%X err %d",
                            spec->key.str(), ms_ifindex, ret_status);
            return pds_ms_api_to_sdk_ret (ret_status);
        }
    
        SDK_TRACE_DEBUG ("Intf create for UUID %s successfully processed",
                          spec->key.str());
    } catch (const Error& e) {
        SDK_TRACE_ERR ("Interface %s creation failed %s", 
                        spec->key.str(), e.what());
        return e.rc();
    }
    return SDK_RET_OK;
}

sdk_ret_t
interface_delete (pds_if_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    uint32_t ms_ifindex;
    types::ApiStatus ret_status;
    
    SDK_ASSERT((spec->type == PDS_IF_TYPE_L3 ||
                spec->type == PDS_IF_TYPE_LOOPBACK));

    try {
        // Fill MS IfIndex from UUID cache
        ms_ifindex = interface_uuid_fetch(spec->key, true);
        
        ret_status = process_interface_update(spec, ms_ifindex,
                                              AMB_ROW_DESTROY);
        if (ret_status != types::ApiStatus::API_STATUS_OK) {
            SDK_TRACE_ERR ("Failed to process interface UUID %s "
                           "MS-Interface 0x%X "
                           "delete err %d", 
                            spec->key.str(), ms_ifindex, ret_status);
            return pds_ms_api_to_sdk_ret (ret_status);
        }
    
        SDK_TRACE_DEBUG ("Intf delete for UUID %s successfully processed",
                         spec->key.str());
    } catch (const Error& e) {
        SDK_TRACE_ERR ("Interface %s deletion failed %s", 
                        spec->key.str(), e.what());
        return e.rc();
    }
    return SDK_RET_OK;
}

sdk_ret_t
interface_update (pds_if_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    uint32_t ms_ifindex;
    types::ApiStatus ret_status;
    
    try {
        // Fill MS IfIndex from UUID cache
        ms_ifindex = interface_uuid_fetch(spec->key, false);
        // Only L3 interface address can be updated. Send the current
        // interface address to Metaswitch. If address is same then metaswitch
        // will treat it as a no-ip
        ret_status = process_interface_update(spec, ms_ifindex,
                                              AMB_ROW_ACTIVE);
        if (ret_status != types::ApiStatus::API_STATUS_OK) {
            SDK_TRACE_ERR ("Failed to process interface UUID %s "
                           "MS-Interface 0x%X "
                           "update err %d", 
                            spec->key.str(), ms_ifindex, ret_status);
            return pds_ms_api_to_sdk_ret (ret_status);
        }
    
        SDK_TRACE_DEBUG ("Intf update for UUID %s successfully processed",
                          spec->key.str());
    } catch (const Error& e) {
        SDK_TRACE_ERR ("Interface %s update failed %s", 
                        spec->key.str(), e.what());
        return e.rc();
    }
    return SDK_RET_OK;
}

};    // namespace pds_ms

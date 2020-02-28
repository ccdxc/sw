// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_subnet.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_interface.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_ctm.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_uuid_obj.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/mgmt/pds_ms_internal_utils_gen.hpp"
#include "gen/proto/internal.pb.h"
#include "nic/metaswitch/stubs/common/pds_ms_ifindex.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_l2f_bd.hpp"
#include "nic/apollo/api/utils.hpp"
#include "nic/sdk/include/sdk/if.hpp"

//---------------------------------------------------------------------
// 2 ways in which HAL is updated -
//
// a) Configure Metaswitch MIB - SlowPath update to HAL
//    Metaswitch will process the MIB config and asynchronously call
//    Metaswitch Stub APIs depending on controlplane state machine.
//    PDS HAL APIs are invoked in async completion mode from the
//    Metaswitch Stub APIs.
//
// b) Invoke PDS API - Fastpath update to HAL
//    This is similar to how all non-controlplane objects are sent to HAL.
//   PDS HAL APIs are invoked in synchronous completion mode.
//
// Subnet Create and Delete operations are always handled as Slowpath updates.
//
// Subnet Update operation uses different mechanism for different fields
// based on the field's dependecies -
//
// a) Owned by Metaswitch (Slowpath update to HAL) -
//    Fields that have dependencies to/from other Metaswitch HAL objects.
//    These fields will be updated to HAL by Metaswitch Stub based on
//    control plane state machine and should NOT be modified in a
//    direct Fastpath update to HAL
//         i) VNI
//        ii) HostIfIndex - LIF bind/unbind to subnet
//
// b) Owned by PDS HAL (Fastpath update to HAL) -
//    Fields that have dependencies to/from non-controlplane PDS HAL objects.
//    Or fields that are unknown to Metaswitch.
//    Updates to these fields need to be directly sent to HAL immediately
//    since these fields may be references to other HAL objects that are
//    sent to HAL (Fastpath updated) directly from the PDSA SVC.
//        i) Policy references,
//       ii) Route table references
//      iii) TOS
//       iv) Virtual-MAC
//    Metaswitch Stub APIs should use the latest value of these fields
//    when invoking PDS HAL APIs in the slowpath.
//
// c) Owned by PDS HAL, also known to Metaswitch (Fastpath update to HAL +
//                                                Metaswitch MIB condig) -
//    Fields that are known to Metaswitch and HAL but do not have
//    dependencies to/from other Metaswitch HAL objects.
//    Hence the HAL programming for these fields need not be in lock-step
//    with Metaswitch controlplane as long as they eventually converge.
//         i) Subnet gateway IP/prefix
//              Used by Metaswitch to advertise as BGP route to other TEPs.
//              Used by HAL/VPP to respond to host ping
//    Metaswitch Stub APIs should use the latest value of these fields
//    when invoking PDS HAL APIs in the slowpath.
//
// Assumptions -
// 1) Subnet's VPC-ID cannot be modified - existing subnet will be deleted and
//    a new subnet created under new VPC.
//
// 2) Since Subnet Delete is always driven through Metaswitch
//    HAL subnet delete will be delayed until Metaswitch state machines
//    clean up all dependent objects.
//    Fields in Subnet Spec that are references to other Fastpath updated
//    objects need to removed in an explicit Update from the upper layer
//    (NetAgent/NPM) before Subnet Delete.
//--------------------------------------------------------------------

namespace pds_ms {

static void
populate_evpn_bd_spec (pds_subnet_spec_t *subnet_spec,
                       uint32_t          bd_id,
                       pds::EvpnBdSpec&  req)
{
    req.set_entityindex (PDS_MS_EVPN_ENT_INDEX);
    req.set_eviindex (bd_id);
    req.set_vni (subnet_spec->fabric_encap.val.vnid);
}

static void
populate_lim_irb_spec (pds_subnet_spec_t     *subnet_spec,
                       uint32_t          bd_id,
                       pds::LimGenIrbIfSpec& req)
{
    req.set_entityindex (PDS_MS_LIM_ENT_INDEX);
    req.set_bdindex (bd_id);
    req.set_bdtype (AMB_LIM_BRIDGE_DOMAIN_EVPN);
    req.set_macaddress (subnet_spec->vr_mac, ETH_ADDR_LEN);
}

static void
populate_lim_irb_if_cfg_spec (pds_subnet_spec_t          *subnet_spec,
                              pds::LimInterfaceCfgSpec&  req,
                              uint32_t                   if_index)
{
    std::string vrf_name;

    // Convert VRF ID to name
    auto mgmt_ctxt = mgmt_state_t::thread_context();
    auto uuid_obj = mgmt_ctxt.state()->lookup_uuid(subnet_spec->vpc);

    // TODO When there is a failure here in IRB then the BD has to be reverted
    if (uuid_obj == nullptr) {
        throw Error(std::string("Subnet has unknown VPC reference ")
                    .append(subnet_spec->vpc.str()), SDK_RET_INVALID_ARG);
    }
    if (uuid_obj->obj_type() != uuid_obj_type_t::VPC) {
        throw Error(std::string("Subnet has invalid VPC reference ")
                    .append(subnet_spec->vpc.str()).append(" containing ")
                    .append(uuid_obj->str()), SDK_RET_INVALID_ARG);
    }
    auto vrf_id = ((vpc_uuid_obj_t*)uuid_obj)->ms_id();
    vrf_name = std::to_string (vrf_id);

    PDS_TRACE_DEBUG("IRB Interface:: VRF ID: %d MSIfIndex: 0x%X VRF name %s len %d",
                    vrf_id, if_index, vrf_name.c_str(), vrf_name.length());

    req.set_entityindex (PDS_MS_LIM_ENT_INDEX);
    req.set_ifindex (if_index);
    req.set_ifenable (AMB_TRUE);
    req.set_ipv4enabled (AMB_TRISTATE_TRUE);
    req.set_ipv4fwding (AMB_TRISTATE_TRUE);
    req.set_ipv6enabled (AMB_TRISTATE_TRUE);
    req.set_ipv6fwding (AMB_TRISTATE_TRUE);
    req.set_fwdingmode (AMB_LIM_FWD_MODE_L3);
    req.set_vrfname (vrf_name);
}

static void
populate_evpn_if_bing_cfg_spec (pds_subnet_spec_t        *subnet_spec,
                                pds::EvpnIfBindCfgSpec&  req,
                                uint32_t                 bd_id,
                                uint32_t                 if_index)
{
    req.set_entityindex (PDS_MS_EVPN_ENT_INDEX);
    req.set_ifindex (if_index);
    req.set_eviindex (bd_id);
}


static void
populate_lim_swif_cfg_spec (pds::LimInterfaceCfgSpec& req,
                            uint32_t                  if_index)
{
    req.set_entityindex (PDS_MS_LIM_ENT_INDEX);
    req.set_ifindex (if_index);
    req.set_ifenable (AMB_TRUE);
    req.set_ipv4enabled (AMB_TRISTATE_FALSE);
    req.set_ipv4fwding (AMB_TRISTATE_FALSE);
    req.set_ipv6enabled (AMB_TRISTATE_FALSE);
    req.set_ipv6fwding (AMB_TRISTATE_FALSE);
    req.set_fwdingmode (AMB_LIM_FWD_MODE_L2);
}

static void
populate_lim_soft_if_spec (pds::LimInterfaceSpec& req,
                           pds_ifindex_t          host_ifindex)
{
    req.set_ifid (LIF_IFINDEX_TO_LIF_ID(host_ifindex));
    req.set_iftype (pds::LIM_IF_TYPE_LIF);
}

static types::ApiStatus
process_subnet_update (pds_subnet_spec_t   *subnet_spec,
                       uint32_t             bd_id,
                       NBB_LONG            row_status)
{
    uint32_t if_index;

    PDS_MS_START_TXN(PDS_MS_CTM_GRPC_CORRELATOR);

    // EVPN BD Row Update
    PDS_TRACE_DEBUG("%s Subnet %s BD ID: %d",
                     (row_status == AMB_ROW_DESTROY) ? "Delete":"Create",
                     subnet_spec->key.str(), bd_id);

    pds::EvpnBdSpec evpn_bd_spec;
    populate_evpn_bd_spec (subnet_spec, bd_id, evpn_bd_spec);
    pds_ms_set_amb_evpn_bd (evpn_bd_spec, row_status, PDS_MS_CTM_GRPC_CORRELATOR, FALSE);

    // limGenIrbInterfaceTable Row Update
    pds::LimGenIrbIfSpec irb_spec;
    populate_lim_irb_spec (subnet_spec, bd_id, irb_spec);
    pds_ms_set_amb_lim_gen_irb_if (irb_spec, row_status, PDS_MS_CTM_GRPC_CORRELATOR, FALSE);

    // Get IRB If Index
    if_index = bd_id_to_ms_ifindex (bd_id);
    PDS_TRACE_DEBUG("IRB Interface %s:: BD ID: %d MSIfIndex: 0x%X",
                     (row_status == AMB_ROW_DESTROY) ? "Delete":"Create",
                     bd_id, if_index);

    // Update IRB to VRF binding
    pds::LimInterfaceCfgSpec lim_if_spec;
    populate_lim_irb_if_cfg_spec (subnet_spec, lim_if_spec, if_index);
    pds_ms_set_amb_lim_if_cfg (lim_if_spec, row_status, PDS_MS_CTM_GRPC_CORRELATOR, FALSE);

    // Configure IRB IP Address
    ip_prefix_t ip_prefix;
    ip_prefix.len = subnet_spec->v4_prefix.len;
    ip_prefix.addr.af = IP_AF_IPV4;
    ip_prefix.addr.addr.v4_addr = subnet_spec->v4_prefix.v4_addr;
    pds::LimInterfaceAddrSpec lim_addr_spec;
    populate_lim_addr_spec (&ip_prefix, lim_addr_spec,
                            pds::LIM_IF_TYPE_IRB, bd_id);
    pds_ms_set_amb_lim_l3_if_addr (lim_addr_spec, row_status, PDS_MS_CTM_GRPC_CORRELATOR, FALSE);

    if (!is_pds_obj_key_invalid(subnet_spec->host_if)) {
        // Create Lif here for now
        pds::LimInterfaceSpec lim_swif_spec;
        auto lif_ifindex = api::objid_from_uuid(subnet_spec->host_if);
        populate_lim_soft_if_spec (lim_swif_spec, lif_ifindex);
        pds_ms_set_amb_lim_software_if (lim_swif_spec, row_status, PDS_MS_CTM_GRPC_CORRELATOR, FALSE);

        // Get Lif's MS IfIndex
        if_index = pds_to_ms_ifindex (lif_ifindex, IF_TYPE_LIF);
        PDS_TRACE_DEBUG ("SW Interface %s:: LIF UUID %s PDS IfIndex: 0x%X MSIfIndex: 0x%X",
                          (row_status == AMB_ROW_DESTROY) ? "Delete":"Create",
                          subnet_spec->host_if.str(), lif_ifindex, if_index);

        // Set Lif interface settings
        pds::LimInterfaceCfgSpec lim_swifcfg_spec;
        populate_lim_swif_cfg_spec (lim_swifcfg_spec, if_index);
        pds_ms_set_amb_lim_if_cfg (lim_swifcfg_spec, row_status, PDS_MS_CTM_GRPC_CORRELATOR, FALSE);

        // evpnIfBindCfgTable Row Update
        pds::EvpnIfBindCfgSpec evpn_if_bind_spec;
        populate_evpn_if_bing_cfg_spec (subnet_spec, evpn_if_bind_spec, bd_id, if_index);
        pds_ms_set_amb_evpn_if_bind_cfg (evpn_if_bind_spec, row_status, PDS_MS_CTM_GRPC_CORRELATOR, FALSE);
    }

    PDS_MS_END_TXN(PDS_MS_CTM_GRPC_CORRELATOR);

    // blocking on response from MS
    return pds_ms::mgmt_state_t::ms_response_wait();
}

struct subnet_upd_flags_t {
    bool bd = false;
    bool bd_if = false;
    bool irb = false;
    operator bool() {
        return (bd || bd_if || irb);
    }
};

static types::ApiStatus
process_subnet_field_update (pds_subnet_spec_t   *subnet_spec,
                             const subnet_upd_flags_t& ms_upd_flags,
                             uint32_t             bd_id,
                             NBB_LONG             row_status)
{
    uint32_t ms_ifindex;

    PDS_MS_START_TXN(PDS_MS_CTM_GRPC_CORRELATOR);

    // EVPN BD Row Update
    if (ms_upd_flags.bd) {
        PDS_TRACE_DEBUG("Subnet %s BD %d Update: Trigger MS BD Update", subnet_spec->key.str(), bd_id);
        pds::EvpnBdSpec evpn_bd_spec;
        populate_evpn_bd_spec (subnet_spec, bd_id, evpn_bd_spec);
        pds_ms_set_amb_evpn_bd (evpn_bd_spec, row_status, PDS_MS_CTM_GRPC_CORRELATOR, FALSE);
    }

    // Create Lif here for now
    if (ms_upd_flags.bd_if) {
        PDS_TRACE_DEBUG("Subnet %s BD %d Update: Trigger MS BD If Update", subnet_spec->key.str(), bd_id);
        pds::LimInterfaceSpec lim_swif_spec;
        auto lif_ifindex = api::objid_from_uuid(subnet_spec->host_if);
        populate_lim_soft_if_spec (lim_swif_spec, lif_ifindex);
        pds_ms_set_amb_lim_software_if (lim_swif_spec, row_status, PDS_MS_CTM_GRPC_CORRELATOR, FALSE);

        // Get Lif's MS IfIndex
        ms_ifindex = pds_to_ms_ifindex (lif_ifindex, IF_TYPE_LIF);
        PDS_TRACE_DEBUG ("SW Interface:: LIF UUID %s PDS IfIndex: 0x%X MSIfIndex: 0x%X",
                         subnet_spec->host_if.str(), lif_ifindex, ms_ifindex);

        // Set Lif interface settings
        pds::LimInterfaceCfgSpec lim_swifcfg_spec;
        populate_lim_swif_cfg_spec (lim_swifcfg_spec, ms_ifindex);
        pds_ms_set_amb_lim_if_cfg (lim_swifcfg_spec, row_status, PDS_MS_CTM_GRPC_CORRELATOR, FALSE);

        // evpnIfBindCfgTable Row Update
        pds::EvpnIfBindCfgSpec evpn_if_bind_spec;
        populate_evpn_if_bing_cfg_spec (subnet_spec, evpn_if_bind_spec, bd_id, ms_ifindex);
        pds_ms_set_amb_evpn_if_bind_cfg (evpn_if_bind_spec, row_status, PDS_MS_CTM_GRPC_CORRELATOR, FALSE);
    }

    if (ms_upd_flags.irb) {
        PDS_TRACE_DEBUG("Subnet %s BD %d Update: Trigger MS IRB Update", subnet_spec->key.str(), bd_id);
        // Configure IRB IP Address
        ip_prefix_t ip_prefix;
        ip_prefix.len = subnet_spec->v4_prefix.len;
        ip_prefix.addr.af = IP_AF_IPV4;
        ip_prefix.addr.addr.v4_addr = subnet_spec->v4_prefix.v4_addr;
        pds::LimInterfaceAddrSpec lim_addr_spec;
        populate_lim_addr_spec (&ip_prefix, lim_addr_spec,
                                pds::LIM_IF_TYPE_IRB, bd_id);
        pds_ms_set_amb_lim_l3_if_addr (lim_addr_spec, row_status, PDS_MS_CTM_GRPC_CORRELATOR, FALSE);
    }

    PDS_MS_END_TXN(PDS_MS_CTM_GRPC_CORRELATOR);

    // blocking on response from MS
    return pds_ms::mgmt_state_t::ms_response_wait();
}

enum class pds_ms_subnet_cache_op_t {
    CREATE,
    MARK_DEL,
    REVERT_MARK_DEL,
    COMMIT_DEL
};

static bool
cache_subnet_spec(pds_subnet_spec_t* spec, uint32_t bd_id, pds_ms_subnet_cache_op_t op)
{
    bool cleanup = false;

    auto state_ctxt = state_t::thread_context();
    switch (op) {
    case pds_ms_subnet_cache_op_t::CREATE:
    {    
        auto subnet_obj = state_ctxt.state()->subnet_store().get(bd_id);
        if ((subnet_obj != nullptr) &&
            (spec->key != subnet_obj->spec().key)) {
            throw Error(std::string("Another subnet ")
                        .append(subnet_obj->spec().key.str())
                        .append(" already exists with same internal BD ID ")
                        .append(std::to_string(bd_id)), SDK_RET_ERR);
        }
        subnet_obj_uptr_t subnet_obj_uptr (new subnet_obj_t(*spec));
        state_ctxt.state()->subnet_store().add_upd(bd_id, std::move(subnet_obj_uptr));
        break;
    }
    case pds_ms_subnet_cache_op_t::MARK_DEL:
    {
        auto subnet_obj = state_ctxt.state()->subnet_store().get(bd_id);
        if (subnet_obj == nullptr) {
            return cleanup;
        }
        subnet_obj->properties().spec_invalid = true;
        break;
    }
    case pds_ms_subnet_cache_op_t::REVERT_MARK_DEL:
    {
        auto subnet_obj = state_ctxt.state()->subnet_store().get(bd_id);
        if (subnet_obj == nullptr) {
            return cleanup;
        }
        subnet_obj->properties().spec_invalid = false;
        break;
    }
    case pds_ms_subnet_cache_op_t::COMMIT_DEL:
        cleanup = true;
        auto subnet_obj = state_ctxt.state()->subnet_store().get(bd_id);
        if (subnet_obj == nullptr) {
            return cleanup;
        }
        if (subnet_obj->properties().hal_created) {
            // Let L2F BD stub release the UUID
            cleanup = false;
        }
        state_ctxt.state()->subnet_store().erase(bd_id);
        break;
    }
    return cleanup;
}

static ms_bd_id_t
subnet_uuid_2_idx_alloc (const pds_obj_key_t& key)
{
    auto mgmt_ctxt = mgmt_state_t::thread_context();
    // Subnet Create - Allocate a new index
    auto uuid_obj = mgmt_ctxt.state()->lookup_uuid(key);
    if (uuid_obj != nullptr) {
        throw Error(std::string("Subnet Create for existing UUID ")
                    .append(key.str()).append(" containing ")
                    .append(uuid_obj->str()), SDK_RET_ENTRY_EXISTS);
    }
    subnet_uuid_obj_uptr_t subnet_uuid_obj (new subnet_uuid_obj_t(key));
    auto bd_id = subnet_uuid_obj->ms_id();
    mgmt_ctxt.state()->set_pending_uuid_create(key,
                                               std::move(subnet_uuid_obj));
    return bd_id;
}

static ms_bd_id_t
subnet_uuid_2_idx_fetch (const pds_obj_key_t& key, bool del_op)
{
    // Update or Delete - fetch the BD ID
    auto mgmt_ctxt = mgmt_state_t::thread_context();
    auto uuid_obj = mgmt_ctxt.state()->lookup_uuid(key);
    if (uuid_obj == nullptr) {
        throw Error(std::string("Unknown UUID in Subnet Request ")
                    .append(key.str()), SDK_RET_ENTRY_NOT_FOUND);
    }
    if (uuid_obj->obj_type() != uuid_obj_type_t::SUBNET) {
        throw Error(std::string("Wrong UUID ").append(key.str())
                    .append(" containing ").append(uuid_obj->str())
                    .append(" in Subnet request"), SDK_RET_INVALID_ARG);
    }
    auto subnet_uuid_obj = (subnet_uuid_obj_t*) uuid_obj;
    PDS_TRACE_DEBUG("Fetched Subnet UUID %s = BD %d",
                     key.str(), subnet_uuid_obj->ms_id());
    if (del_op) {
        mgmt_ctxt.state()->set_pending_uuid_delete(key);
    }
    return subnet_uuid_obj->ms_id();
}

sdk_ret_t
subnet_create (pds_subnet_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    types::ApiStatus ret_status;
    ms_bd_id_t bd_id = 0;

    try {
        bd_id = subnet_uuid_2_idx_alloc(spec->key);
        cache_subnet_spec (spec, bd_id, pds_ms_subnet_cache_op_t::CREATE);

        ret_status = process_subnet_update (spec, bd_id, AMB_ROW_ACTIVE);
        if (ret_status != types::ApiStatus::API_STATUS_OK) {
            PDS_TRACE_ERR ("Failed to process subnet %s bd %d create (error=%d)",
                           spec->key.str(), bd_id, ret_status);

            // Delete the cached subnet spec
            cache_subnet_spec (spec, bd_id, pds_ms_subnet_cache_op_t::COMMIT_DEL);
            return pds_ms_api_to_sdk_ret (ret_status);
        }
        PDS_TRACE_DEBUG ("Subnet %s bd %d create is successfully processed",
                         spec->key.str(), bd_id);
    } catch (const Error& e) {
        PDS_TRACE_ERR ("Subnet %s creation failed %s",
                        spec->key.str(), e.what());
        // Internal BD ID already released - Delete the cached subnet spec
        if (bd_id != 0) {
            cache_subnet_spec (spec, bd_id, pds_ms_subnet_cache_op_t::COMMIT_DEL);
        }
        return e.rc();
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_delete (pds_subnet_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    types::ApiStatus ret_status;
    ms_bd_id_t bd_id = 0;
    bool delete_completed = false;

    try {
        bd_id = subnet_uuid_2_idx_fetch(spec->key, true);
        // Mark as deleted so that L2F stub can release the Subnet UUID
        cache_subnet_spec (spec, bd_id, pds_ms_subnet_cache_op_t::MARK_DEL);

        ret_status = process_subnet_update (spec, bd_id, AMB_ROW_DESTROY);
        if (ret_status != types::ApiStatus::API_STATUS_OK) {
            cache_subnet_spec (spec, bd_id,
                               pds_ms_subnet_cache_op_t::REVERT_MARK_DEL);

            PDS_TRACE_ERR ("Failed to process subnet %s bd %d delete (error=%d)",
                           spec->key.str(), bd_id, ret_status);
            return pds_ms_api_to_sdk_ret (ret_status);
        }
        delete_completed = true;

        if (cache_subnet_spec (spec, bd_id, pds_ms_subnet_cache_op_t::COMMIT_DEL)) {
            // Subnet UUID is released by the L2F BD stub usually.
            // But if L2F BD stub was never invoked for this subnet
            // for whatever reason then release the UUID here
            PDS_TRACE_DEBUG ("MS BD %d UUID %s Release", bd_id, spec->key.str());
            auto mgmt_ctxt = mgmt_state_t::thread_context();
            mgmt_ctxt.state()->remove_uuid(spec->key);
        }
        PDS_TRACE_DEBUG ("subnet %s bd %d delete is successfully processed",
                         spec->key.str(), bd_id);

    } catch (const Error& e) {
        PDS_TRACE_ERR ("Subnet %s deletion failed %s",
                        spec->key.str(), e.what());
        if (bd_id != 0 && !delete_completed) {
            cache_subnet_spec (spec, bd_id,
                               pds_ms_subnet_cache_op_t::REVERT_MARK_DEL);
        }
        return e.rc();
    }
    return SDK_RET_OK;
}

static void
parse_subnet_update (pds_subnet_spec_t *spec, ms_bd_id_t bd_id,
                     subnet_upd_flags_t& ms_upd_flags)
{
    bool fastpath = false;
    // Enter thread-safe context to access/modify global state
    auto state_ctxt = state_t::thread_context();
    auto subnet_obj = state_ctxt.state()->subnet_store().get(bd_id);
    if (subnet_obj == nullptr) {
        throw Error(std::string("Store lookup failed for subnet ")
                    .append(spec->key.str()).append(" bd ")
                    .append(std::to_string(bd_id)), SDK_RET_ENTRY_NOT_FOUND);
    }

    auto& state_pds_spec = subnet_obj->spec();
    if (memcmp (&state_pds_spec.fabric_encap, &spec->fabric_encap,
                sizeof(state_pds_spec.fabric_encap)) != 0) {
        ms_upd_flags.bd = true;
        PDS_TRACE_INFO("Subnet %s BD %d VNI change - Old %d New %d",
                       spec->key.str(), bd_id,
                       state_pds_spec.fabric_encap.val.vnid,
                       spec->fabric_encap.val.vnid);
        state_pds_spec.fabric_encap = spec->fabric_encap;
    }
    if (state_pds_spec.host_if != spec->host_if) {
        ms_upd_flags.bd_if = true;
        PDS_TRACE_INFO("Subnet %s BD %d Host If change - Old %s New %s",
                       spec->key.str(), bd_id, state_pds_spec.host_if.str(),
                       spec->host_if.str());
        state_pds_spec.host_if = spec->host_if;
    }
    if (state_pds_spec.v4_vr_ip != spec->v4_vr_ip) {
        ms_upd_flags.irb = true;
        // Diff in IP address needs to be driven through fast and slowpath
    }
    // Diff in any other property needs to be driven through fastpath
    if (memcmp(&state_pds_spec, spec, sizeof(*spec)) != 0) {
        PDS_TRACE_INFO("Subnet %s BD %d fastpath parameter change",
                       spec->key.str(), bd_id);
        fastpath = true;
    }
    // Update the cached subnet spec with the new info
    state_pds_spec = *spec;

    if (fastpath) {
        // Stub takes care of sequencing if create has not yet been
        // received from MS.
        auto ret = l2f_bd_update_pds_synch(std::move(state_ctxt),
                                           bd_id, subnet_obj);
        // Do not state_ctxt has been released above
        // Do not access global state beyond this
        if (ret != SDK_RET_OK) {
            throw Error(std::string("Failed to update fastpath fields for Subnet ")
                        .append(spec->key.str()).append(" BD ")
                        .append(std::to_string(bd_id)), ret);
        }
    }
}

sdk_ret_t
subnet_update (pds_subnet_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    subnet_upd_flags_t  ms_upd_flags;
    types::ApiStatus ret_status;
    ms_bd_id_t bd_id;

    try {
        bd_id = subnet_uuid_2_idx_fetch(spec->key, false);
        parse_subnet_update(spec, bd_id, ms_upd_flags);

        if (ms_upd_flags) {
            ret_status = process_subnet_field_update(spec, ms_upd_flags, bd_id,
                                                     AMB_ROW_ACTIVE);
            if (ret_status != types::ApiStatus::API_STATUS_OK) {
                PDS_TRACE_ERR ("Failed to process subnet %s field update err %d",
                               spec->key.str(), ret_status);
                return pds_ms_api_to_sdk_ret (ret_status);
            }
            PDS_TRACE_DEBUG ("Subnet %s field update successfully processed",
                             spec->key.str());
        }
    } catch (const Error& e) {
        PDS_TRACE_ERR ("Subnet %s update failed %s",
                        spec->key.str(), e.what());
        return e.rc();
    }

    return SDK_RET_OK;
}
};    // namespace pds_ms

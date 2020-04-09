// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
#include "nic/metaswitch/stubs/mgmt/pds_ms_vpc.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_ctm.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/mgmt/pds_ms_internal_utils_gen.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_vpc_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_li_vrf.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"

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
//    PDS HAL APIs are invoked in synchronous completion mode.
//
// VPC Create and Delete operations are always handled as Slowpath updates.
//
// VPC Update operation uses different mechanism for different fields
// based on the field's dependecies -
//
// a) Owned by Metaswitch (Slowpath update to HAL) -
//    Fields that have dependencies to/from other Metaswitch HAL objects.
//    These fields will be updated to HAL by Metaswitch Stub based on
//    control plane state machine and should NOT be modified in a
//    direct Fastpath update to HAL
//       -  Currently other MS HAL objects only depeend on the presence
//          of the VRF but not on any specific field in the VPC spec.
//
// b) Owned by PDS HAL (Fastpath update to HAL) -
//    Fields that have dependencies to/from non-controlplane PDS HAL objects.
//    Or fields that are unknown to Metaswitch.
//    Updates to these fields are directly sent to HAL immediately
//    since these fields may be references to other HAL objects that are
//    sent to HAL (Fastpath updated) directly from the PDSA SVC.
//        i) IP Prefix
//       ii) Route table references
//      iii) TOS
//       iv) Virtual-MAC
//    Metaswitch Stub APIs should use the latest value of these fields
//    in the spec when invoking PDS HAL APIs in the slowpath.
//
// c) Owned by PDS HAL, also known to Metaswitch (Fastpath update to HAL +
//                                                Metaswitch MIB condig) -
//    Fields that are known to Metaswitch and HAL but do not have
//    dependencies to/from other Metaswitch HAL objects.
//    Hence the HAL programming for these fields need not be in lock-step
//    with Metaswitch controlplane as long as they eventually converge.
//    They can be updated in parallel to both HAL and Metaswitch.
//         i) VRF VNI (Local node's VNI)
//              Used by Metaswitch to encode Type-5 or Symmetric Type-2
//              BGP EVPN routes to other TEPs. This is unused in our case.
//              In our usecase all VNI HAL programming for remote Type-5 routes
//              is driven by the L3 VXLAN Port MS HAL object which drives the
//              VNI directly in the TEP entry.
//              VRF VNI field belongs to a separate MS MIB table and hence is
//              driven from a separate Proto - EvpnIpVrf. So this field need
//              not be considered for VPC update.
//
//  Hence VPC updates are always Fastpath ONLY.
//
// Assumptions -
// 1) Since VPC Delete is always driven through Metaswitch
//    HAL VPC delete will be delayed until Metaswitch state machines
//    clean up all dependent objects.
//    Fields in VPC Spec that are references to other Fastpath updated
//    objects need to removed in an explicit Update from the upper layer
//    (NetAgent/NPM) before VPC Delete.
//--------------------------------------------------------------------

namespace pds_ms {

static void
populate_lim_vrf_spec (ms_vrf_id_t vrf_id,
                       LimVrfSpec& req)
{
    std::string vrf_name;

    // Convert VRF ID to name
    vrf_name = std::to_string (vrf_id);

    req.set_entityindex (PDS_MS_LIM_ENT_INDEX);
    req.set_vrfname (vrf_name);
    req.set_vrfnamelen (vrf_name.length());
}

static bool
pds_cache_vpc_spec (pds_vpc_spec_t *vpc_spec, ms_vrf_id_t vrf_id,
                              bool op_delete)
{
    bool cleanup = true;
    auto state_ctxt = pds_ms::state_t::thread_context();

    if (op_delete) {
        auto vpc_obj = state_ctxt.state()->vpc_store().get(vrf_id);
        if (vpc_obj == nullptr) {return cleanup;}
        if (vpc_obj->properties().hal_created) {
            PDS_TRACE_DEBUG("VPC %d already created in HAL - marking for delete",
                            vrf_id);
            vpc_obj->properties().spec_invalid = true;
            // Let LI VRF stub release the UUID
            cleanup = false;
        } else {
            PDS_TRACE_DEBUG("VPC %d not created in HAL yet - remove from store",
                            vpc_obj->properties().vrf_id);
            state_ctxt.state()->vpc_store().erase(vpc_obj->properties().vrf_id);
        }
        return cleanup;
    }
    auto vpc_obj = state_ctxt.state()->vpc_store().get(vrf_id);
    if ((vpc_obj != nullptr) &&
        (vpc_spec->key != vpc_obj->properties().vpc_spec.key)) {
        throw Error(std::string("Another VPC ")
                    .append(vpc_obj->properties().vpc_spec.key.str())
                    .append(" already exists with same internal VRF ID ")
                    .append(std::to_string(vrf_id)), SDK_RET_ERR);
    }
    vpc_obj = new pds_ms::vpc_obj_t(vrf_id, *vpc_spec);
    state_ctxt.state()->vpc_store().add_upd(vrf_id, vpc_obj);
    return cleanup;
}

static types::ApiStatus
process_vpc_update (ms_vrf_id_t    vrf_id,
                    mib_idx_t      rtm_index,
                    NBB_LONG       row_status)
{
    PDS_MS_START_TXN(PDS_MS_CTM_GRPC_CORRELATOR);

    // Create new instance of RTM and initiate Joins
    PDS_TRACE_INFO("%s VRF RTM instance %d",
                   (row_status==AMB_ROW_DESTROY) ? "Deleting" : "Creating",
                   rtm_index);

    if (row_status != AMB_ROW_DESTROY) {
        pds_ms_config_t   conf = {0};
        conf.correlator  = PDS_MS_CTM_GRPC_CORRELATOR;
        conf.row_status  = row_status;

        // TODO: For v6 use 64K + rtm_index since vrf_id / rtm_index is capped at 16 bits
        pds_ms_rtm_create(&conf, rtm_index, false);
        pds_ms_evpn_rtm_join(&conf, rtm_index);
    }
    // LIM VRF Row Update
    LimVrfSpec lim_vrf_spec;
    populate_lim_vrf_spec (vrf_id, lim_vrf_spec);
    pds_ms_set_limvrfspec_amb_lim_vrf (lim_vrf_spec, row_status,
                                       PDS_MS_CTM_GRPC_CORRELATOR, FALSE);

    PDS_MS_END_TXN(PDS_MS_CTM_GRPC_CORRELATOR);

    // blocking on response from MS
    return pds_ms::mgmt_state_t::ms_response_wait();
}

static bool overlay_routing_enabled_()
{
    auto mgmt_ctxt = mgmt_state_t::thread_context();
    return mgmt_ctxt.state()->overlay_routing_en();
}

static types::ApiStatus
process_underlay_vpc_create ()
{
    pds_ms_config_t   conf = {0};
    conf.correlator  = PDS_MS_CTM_GRPC_CORRELATOR;
    conf.row_status  = AMB_ROW_ACTIVE;

    // Create new instance of RTM and initiate Joins
    PDS_TRACE_INFO("Underlay VPC creation request received");

    PDS_MS_START_TXN(PDS_MS_CTM_GRPC_CORRELATOR);

    // Underlay only control-plane model requires user configured
    // VXLAN Tunnels to be stitched to underlay nexthop group.
    // Until the Metaswitch support for configured VXLAN tunnels
    // is available the stop-gap solution is to push underlay routes to
    // HAL API thread and have it stitch the TEPs to the Underlay NH groups.
    // This requires even default RTM component to join the FTM component.
    if (!overlay_routing_enabled_()) { 
        PDS_TRACE_DEBUG("Overlay Routing disabled, enable underlay routes to HAL");
        pds_ms_rtm_ftm_join(&conf, PDS_MS_RTM_DEF_ENT_INDEX);
    }
    pds_ms_ft_stub_create (&conf);    // FT stub
    pds_ms_rtm_ft_stub_join (&conf, PDS_MS_RTM_DEF_ENT_INDEX);

    PDS_MS_END_TXN(PDS_MS_CTM_GRPC_CORRELATOR);

    // blocking on response from MS
    return pds_ms::mgmt_state_t::ms_response_wait();
}

static std::pair<ms_vrf_id_t,mib_idx_t>
vpc_uuid_2_idx_alloc (const pds_vpc_spec_t* spec)
{
    auto& key = spec->key;
    auto mgmt_ctxt = mgmt_state_t::thread_context();
    // VPC Create - Allocate a new index
    auto uuid_obj = mgmt_ctxt.state()->lookup_uuid(key);
    if (uuid_obj != nullptr) {
        throw Error(std::string("VPC Create for existing UUID ")
                    .append(key.str()).append(" containing ")
                    .append(uuid_obj->str()), SDK_RET_ENTRY_EXISTS);
    }
    // VPC may not always have a RouteTable ID associated
    if (!is_pds_obj_key_invalid (spec->v4_route_table)) {
        auto rttbl_uuid_obj = mgmt_ctxt.state()->lookup_uuid(spec->v4_route_table);
        if (rttbl_uuid_obj != nullptr) {
            throw Error(std::string("VPC Create cannot use existing Route Table UUID ")
                        .append(spec->v4_route_table.str()).append(" containing ")
                        .append(rttbl_uuid_obj->str()), SDK_RET_ENTRY_EXISTS);
        }
    }
    vpc_uuid_obj_uptr_t vpc_uuid_obj (new vpc_uuid_obj_t
                                      (key, spec->type == PDS_VPC_TYPE_UNDERLAY));
    auto ret = std::make_pair(vpc_uuid_obj->ms_id(),
                              vpc_uuid_obj->ms_v4_rttbl_id());
    mgmt_ctxt.state()->set_pending_uuid_create(spec->key,
                                               std::move(vpc_uuid_obj));
    // TODO Store the Route Table UUID to RTM index mapping
    // if we need overlay Route Table Get in future
    return ret;
}

static std::pair<ms_vrf_id_t,mib_idx_t>
vpc_uuid_2_idx_fetch (const pds_obj_key_t& key, bool del_op)
{
    // Update or Delete - fetch the BD ID
    auto mgmt_ctxt = mgmt_state_t::thread_context();
    auto uuid_obj = mgmt_ctxt.state()->lookup_uuid(key);
    if (uuid_obj == nullptr) {
        throw Error(std::string("Unknown UUID in VPC Request ")
                    .append(key.str()), SDK_RET_ENTRY_NOT_FOUND);
    }
    if (uuid_obj->obj_type() != uuid_obj_type_t::VPC) {
        throw Error(std::string("Wrong UUID ").append(key.str())
                    .append(" containing ").append(uuid_obj->str())
                    .append(" in VPC request"), SDK_RET_INVALID_ARG);
    }
    auto vpc_uuid_obj = (vpc_uuid_obj_t*) uuid_obj;
    PDS_TRACE_DEBUG("Fetched VPC UUID %s = VRF %d",
                     key.str(), vpc_uuid_obj->ms_id());
    if (del_op) {
        mgmt_ctxt.state()->set_pending_uuid_delete(key);
    }
    return std::make_pair(vpc_uuid_obj->ms_id(),
                          vpc_uuid_obj->ms_v4_rttbl_id());
}

sdk_ret_t
vpc_create (pds_vpc_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    types::ApiStatus ret_status;
    ms_vrf_id_t vrf_id = 0;

    // lock to allow only one grpc thread processing at a time
    std::lock_guard<std::mutex> lck(pds_ms::mgmt_state_t::grpc_lock());

    try {
        // Guard to release all pending UUIDs in case of any failures
        mgmt_uuid_guard_t uuid_guard;

        mib_idx_t   rtm_index;
        std::tie(vrf_id,rtm_index) = vpc_uuid_2_idx_alloc(spec);
        // cache VPC spec to be used in hals stub
        pds_cache_vpc_spec(spec, vrf_id, false);

        if (spec->type != PDS_VPC_TYPE_UNDERLAY) {
            if (!mgmt_check_vni(spec->fabric_encap.val.vnid,
                                mgmt_obj_type_e::VPC,
                                vrf_id, spec->key)) {
                // Delete the cached spec
                pds_cache_vpc_spec(spec, vrf_id, true);
                return SDK_RET_INVALID_ARG;
            }
            // Create VRF in Metaswitch and send component Join
            // Metaswitch invokes LI Stub which will push the VPC to HAL
            ret_status = process_vpc_update (vrf_id, rtm_index, AMB_ROW_ACTIVE);
            if (ret_status != types::ApiStatus::API_STATUS_OK) {
                PDS_TRACE_ERR ("Failed to process VPC %s VRF %d create (error=%d)",
                               spec->key.str(), vrf_id, ret_status);
                // Delete the cached spec
                pds_cache_vpc_spec(spec, vrf_id, true);
                return pds_ms_api_to_sdk_ret (ret_status);
            }
            mgmt_set_vni(spec->fabric_encap.val.vnid,
                         mgmt_obj_type_e::VPC, vrf_id, spec->key);
        } else {
            // Default VRF is already created in Metaswitch by default
            // Send component Join and manually push the VPC to HAL
            ret_status = process_underlay_vpc_create();
            if (ret_status != types::ApiStatus::API_STATUS_OK) {
                PDS_TRACE_ERR ("Failed to process underlay VPC %s create (error=%d)",
                               spec->key.str(), ret_status);
                // Delete the cached spec
                pds_cache_vpc_spec(spec, vrf_id, true);
                return pds_ms_api_to_sdk_ret (ret_status);
            }
            auto ret = li_vrf_underlay_vpc_commit_pds_synch(*spec, true);
            if (ret != SDK_RET_OK) {
                PDS_TRACE_ERR("Error commiting Underlay VPC to HAL");
                // Delete the cached spec
                pds_cache_vpc_spec(spec, vrf_id, true);
                return ret;
            }
            // Commit UUID mapping store
            auto mgmt_ctxt = mgmt_state_t::thread_context();
            mgmt_ctxt.state()->commit_pending_uuid();
        }
        PDS_TRACE_DEBUG ("VPC %s VRF %d RouteTable %s RTM-v4 %d created successfully",
                         spec->key.str(), vrf_id, spec->v4_route_table.str(),
                         rtm_index);
    } catch (const Error& e) {
        PDS_TRACE_ERR ("VPC %s create failed %s", spec->key.str(), e.what());
        if (vrf_id != 0) {
            pds_cache_vpc_spec(spec, vrf_id, true);
        }
        return e.rc();
    }
    return SDK_RET_OK;
}

static bool vpc_has_subnets (const pds_obj_key_t& vpc_uuid)
{
    bool found = false;
    auto state_ctxt = state_t::thread_context();

    state_ctxt.state()->subnet_store().
        walk([vpc_uuid, &found] (ms_bd_id_t bd,
                                subnet_obj_t& subnet_obj) -> bool{
        if (subnet_obj.spec().vpc == vpc_uuid) {
            PDS_TRACE_DEBUG ("Found Subnet %s in VPC %s",
                             subnet_obj.spec().key.str(), vpc_uuid.str());
            found = true;
            return false;
        }
        return true;
    });
    return found;
}

sdk_ret_t
vpc_delete (pds_vpc_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    types::ApiStatus ret_status;
    if (spec->type == PDS_VPC_TYPE_UNDERLAY) {
        PDS_TRACE_ERR ("Underlay VPC %s deletion not allowed",
                       spec->key.str());
        return SDK_RET_INVALID_OP;
    }

    // lock to allow only one grpc thread processing at a time
    std::lock_guard<std::mutex> lck(pds_ms::mgmt_state_t::grpc_lock());
    try {
        // Guard to release all pending UUIDs in case of any failures
        mgmt_uuid_guard_t uuid_guard;

        ms_vrf_id_t vrf_id;
        mib_idx_t   rtm_index;
        std::tie(vrf_id,rtm_index) = vpc_uuid_2_idx_fetch(spec->key, true);

        if (vpc_has_subnets(spec->key)) {
            PDS_TRACE_ERR ("Cannot delete VPC %s that has subnets attached to it",
                           spec->key.str());
            return SDK_RET_INVALID_OP;
        }
        ret_status = process_vpc_update (vrf_id, rtm_index, AMB_ROW_DESTROY);
        if (ret_status != types::ApiStatus::API_STATUS_OK) {
            PDS_TRACE_ERR ("Failed to process VPC %s VRF %d delete (error=%d)",
                           spec->key.str(), vrf_id, ret_status);
            return pds_ms_api_to_sdk_ret (ret_status);
        }
        PDS_TRACE_DEBUG ("VPC %s VRF %d delete is successfully processed",
                         spec->key.str(), vrf_id);
        // Remove cached VPC spec, after successful reply from MS
        mgmt_reset_vni(spec->fabric_encap.val.vnid);
        if (pds_cache_vpc_spec(spec, vrf_id, true)) {
            // VPC UUID is released by the LI VRF stub usually.
            // But if LI VRF stub was never invoked for this VPC
            // for whatever reason then release the UUID here
            PDS_TRACE_DEBUG ("MS VRF %d UUID %s Release", vrf_id, spec->key.str());
            auto mgmt_ctxt = mgmt_state_t::thread_context();
            mgmt_ctxt.state()->remove_uuid(spec->key);
        }

    } catch (const Error& e) {
        PDS_TRACE_ERR ("VPC %s deletion failed %s", spec->key.str(), e.what());
        return e.rc();
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_update (pds_vpc_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    // lock to allow only one grpc thread processing at a time
    std::lock_guard<std::mutex> lck(pds_ms::mgmt_state_t::grpc_lock());

    // Enter thread-safe context to access/modify global state
    try {
        // Guard to release all pending UUIDs in case of any failures
        mgmt_uuid_guard_t uuid_guard;

        ms_vrf_id_t vrf_id;
        mib_idx_t   rtm_index;
        std::tie(vrf_id,rtm_index) = vpc_uuid_2_idx_fetch(spec->key, false);

        if (spec->type == PDS_VPC_TYPE_UNDERLAY) {
            auto ret = li_vrf_underlay_vpc_commit_pds_synch(*spec, false);
            if (ret != SDK_RET_OK) {
                PDS_TRACE_ERR("Error commiting Underlay VPC Update to HAL");
                return ret;
            }
            return SDK_RET_OK;
        }
        auto state_ctxt = state_t::thread_context();
        auto vpc_obj = state_ctxt.state()->vpc_store().get(vrf_id);
        if (vpc_obj == nullptr) {
            PDS_TRACE_ERR("VPC update for unknown VRF %d", spec->key.id);
            return SDK_RET_ENTRY_NOT_FOUND;
        }
        // Update the cached vpc spec with the new info
        vpc_obj->properties().vpc_spec = *spec;

        // PDS MS LI stub takes care of sequencing if create has not yet been
        // received from MS. Below function will release the state context lock.
        return li_vrf_update_pds_synch(std::move(state_ctxt), vpc_obj);
    } catch (const Error& e) {
        PDS_TRACE_ERR ("VPC %s update failed %s", spec->key.str(), e.what());
        return e.rc();
    }
}
}    // namespace pds_ms

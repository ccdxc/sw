// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
#include "nic/metaswitch/stubs/mgmt/pds_ms_vpc.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_ctm.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/mgmt/pds_ms_internal_utils_gen.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_vpc_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
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
//    HAL subnet delete will be delayed until Metaswitch state machines
//    clean up all dependent objects.
//    Fields in VPC Spec that are references to other Fastpath updated
//    objects need to removed in an explicit Update from the upper layer
//    (NetAgent/NPM) before VPC Delete.
//--------------------------------------------------------------------

namespace pds_ms {

static void
populate_lim_vrf_spec (pds_vpc_spec_t  *vpc_spec,
                       pds::LimVrfSpec& req)
{
    std::string vrf_name;

    // Convert VRF ID to name
    vrf_name = std::to_string (pdsobjkey2msidx(vpc_spec->key));

    req.set_entityindex (PDS_MS_LIM_ENT_INDEX);
    req.set_vrfname (vrf_name);
    req.set_vrfnamelen (vrf_name.length());
}
static void
pds_cache_vni_to_vrf_mapping (pds_vpc_spec_t *vpc_spec, bool op_delete)
{
    auto state_ctxt = pds_ms::state_t::thread_context();

    if (op_delete) {
        auto vpc_obj = state_ctxt.state()->vpc_store().get(pdsobjkey2msidx(vpc_spec->key));
        if (vpc_obj == nullptr) {return;}
        if (vpc_obj->properties().hal_created) {
            SDK_TRACE_DEBUG("VPC already created in HAL - marking for delete",
                            vpc_spec->key.id);
            vpc_obj->properties().spec_invalid = true;
        } else {
            SDK_TRACE_DEBUG("VPC %d not created in HAL yet - remove from store",
                            vpc_obj->properties().vrf_id);
            state_ctxt.state()->vpc_store().erase(vpc_obj->properties().vrf_id);
        }
        return;
    }
    auto vpc_obj = (new pds_ms::vpc_obj_t(*vpc_spec));
    state_ctxt.state()->vpc_store().add_upd(vpc_obj->properties().vrf_id, vpc_obj);
}

static types::ApiStatus
process_vpc_update (pds_vpc_spec_t *vpc_spec,
                    NBB_LONG       row_status)
{
    PDS_MS_START_TXN(PDS_MS_CTM_GRPC_CORRELATOR);

    // Create new instance of RTM and initiate Joins
    pds_ms_config_t   conf = {0};
    auto entity_index = pdsobjkey2msidx(vpc_spec->key);
    SDK_TRACE_INFO("Creating new VRF MS RTM instance %d", entity_index);
    conf.correlator  = PDS_MS_CTM_GRPC_CORRELATOR;
    conf.row_status  = AMB_ROW_ACTIVE;
    pds_ms_rtm_create(&conf, entity_index, false);
    pds_ms_evpn_rtm_join(&conf, entity_index);

    // LIM VRF Row Update
    pds::LimVrfSpec lim_vrf_spec;
    populate_lim_vrf_spec (vpc_spec, lim_vrf_spec);
    pds_ms_set_amb_lim_vrf (lim_vrf_spec, row_status, PDS_MS_CTM_GRPC_CORRELATOR);

    PDS_MS_END_TXN(PDS_MS_CTM_GRPC_CORRELATOR);

    // blocking on response from MS
    return pds_ms::mgmt_state_t::ms_response_wait();
}

sdk_ret_t
vpc_create (pds_vpc_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    types::ApiStatus ret_status;

    // cache VPC spec to be used in hals stub
    pds_cache_vni_to_vrf_mapping (spec, false);

    ret_status = process_vpc_update (spec, AMB_ROW_ACTIVE);
    if (ret_status != types::ApiStatus::API_STATUS_OK) {
        SDK_TRACE_ERR ("Failed to process vpc %d create (error=%d)\n",
                        spec->key.id, ret_status);
        return pds_ms_api_to_sdk_ret (ret_status);
    }

    SDK_TRACE_DEBUG ("vpc %d create is successfully processed\n",
                      spec->key.id);
    return SDK_RET_OK;
}

sdk_ret_t
vpc_delete (pds_vpc_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    types::ApiStatus ret_status;
    ret_status = process_vpc_update (spec, AMB_ROW_DESTROY);
    if (ret_status != types::ApiStatus::API_STATUS_OK) {
        SDK_TRACE_ERR ("Failed to process vpc %d delete (error=%d)\n",
                        spec->key.id, ret_status);
        return pds_ms_api_to_sdk_ret (ret_status);
    }

    SDK_TRACE_DEBUG ("vpc %d delete is successfully processed\n",
                      spec->key.id);

    // TODO: Do we need to delete the mapping from here or from HAL stubs?
    // Remove cached VPC spec, after successful reply from MS
    pds_cache_vni_to_vrf_mapping(spec, true);

    return SDK_RET_OK;
}

sdk_ret_t
vpc_update (pds_vpc_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    // Enter thread-safe context to access/modify global state
    auto state_ctxt = state_t::thread_context();
    auto vpc_obj = state_ctxt.state()->vpc_store().get(pdsobjkey2msidx(spec->key));
    if (vpc_obj == nullptr) {
        SDK_TRACE_ERR("VPC update for unknown VRF %d", spec->key.id);
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    // Update the cached vpc spec with the new info
    vpc_obj->properties().vpc_spec = *spec;

    // PDS MS LI stub takes care of sequencing if create has not yet been
    // received from MS. Below function will release the state context lock.
    return li_vrf_update_pds_synch(std::move(state_ctxt), vpc_obj);

}
};    // namespace pds_ms

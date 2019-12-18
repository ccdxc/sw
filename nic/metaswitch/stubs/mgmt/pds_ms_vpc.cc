// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
#include "nic/metaswitch/stubs/mgmt/pds_ms_vpc.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_ctm.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/mgmt/pdsa_internal_utils_gen.hpp"
#include "nic/metaswitch/stubs/common/pdsa_vpc_store.hpp"
#include "nic/metaswitch/stubs/common/pdsa_state.hpp"
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
    vrf_name = std::to_string (vpc_spec->key.id);

    req.set_entityindex (PDSA_LIM_ENT_INDEX); 
    req.set_vrfname (vrf_name);
    req.set_vrfnamelen (vrf_name.length());
}

static void
pds_cache_vni_to_vrf_mapping (pds_vpc_spec_t *vpc_spec, bool op_delete)
{
    auto state_ctxt = pdsa_stub::state_t::thread_context();

    if (op_delete) {
        auto vpc_obj = state_ctxt.state()->vpc_store().get(vpc_spec->key.id);
        if (vpc_obj == nullptr) {return;}
        if (vpc_obj->properties().hal_created) {
            vpc_obj->properties().hal_created = false;
        } else {
            state_ctxt.state()->vpc_store().erase(vpc_spec->key.id);
        }
        return;
    } 
    auto vpc_obj = (new pdsa_stub::vpc_obj_t(*vpc_spec));
    state_ctxt.state()->vpc_store().add_upd(vpc_spec->key.id, vpc_obj);
}

static void
process_vpc_update (pds_vpc_spec_t *vpc_spec, 
                    NBB_LONG       row_status)
{
    PDSA_START_TXN(PDSA_CTM_GRPC_CORRELATOR);
    
    // LIM VRF Row Update
    pds::LimVrfSpec lim_vrf_spec;
    populate_lim_vrf_spec (vpc_spec, lim_vrf_spec);
    pdsa_set_amb_lim_vrf (lim_vrf_spec, row_status, PDSA_CTM_GRPC_CORRELATOR);

    // TODO: AMB_EVPN_IP_VRF_RT to configure manual RT

    PDSA_END_TXN(PDSA_CTM_GRPC_CORRELATOR);

    // blocking on response from MS
    pds_ms::mgmt_state_t::ms_response_wait();
   
    // TODO: read return status from ms_response and send to caller
    return ;
}

sdk_ret_t
vpc_create (pds_vpc_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    // cache VPC spec to be used in hals stub
    pds_cache_vni_to_vrf_mapping (spec, false);
    process_vpc_update (spec, AMB_ROW_ACTIVE);

    // TODO: Get correct return code from CTM callback
    return SDK_RET_OK;
}

sdk_ret_t
vpc_delete (pds_vpc_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    process_vpc_update (spec, AMB_ROW_DESTROY);

    // Remove cached VPS spec, after successful reply from MS
    pds_cache_vni_to_vrf_mapping (spec, true);

    // TODO: Get correct return code from CTM callback
    return SDK_RET_OK;
}

sdk_ret_t
vpc_update (pds_vpc_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    // Enter thread-safe context to access/modify global state
    auto state_ctxt = state_t::thread_context();
    auto vpc_obj = state_ctxt.state()->vpc_store().get(spec->key.id);
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

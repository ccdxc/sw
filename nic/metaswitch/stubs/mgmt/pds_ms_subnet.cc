// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_subnet.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_ctm.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/mgmt/pdsa_internal_utils_gen.hpp"
#include "gen/proto/internal.pb.h"
#include "nic/metaswitch/stubs/common/pds_ms_ifindex.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/metaswitch/stubs/common/pdsa_state.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_l2f_bd.hpp"
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
                       pds::EvpnBdSpec&  req)
{
    req.set_entityindex (PDSA_EVPN_ENT_INDEX);
    req.set_eviindex (subnet_spec->key.id);
    req.set_vni (subnet_spec->fabric_encap.val.vnid);
}

static void
populate_lim_irb_spec (pds_subnet_spec_t     *subnet_spec,
                       pds::LimGenIrbIfSpec& req)
{
    req.set_entityindex (PDSA_LIM_ENT_INDEX);
    req.set_bdindex (subnet_spec->key.id);
    req.set_bdtype (AMB_LIM_BRIDGE_DOMAIN_EVPN);
} 

static void
populate_lim_irb_if_cfg_spec (pds_subnet_spec_t          *subnet_spec,
                              pds::LimInterfaceCfgSpec&  req)
{
    uint32_t    if_index = 0;
    std::string vrf_name;

    // Get IRB If Index
    if_index = bd_id_to_ms_ifindex (subnet_spec->key.id);
    SDK_TRACE_DEBUG("IRB Interface:: BD ID: 0x%X MSIfIndex: 0x%X", 
                    subnet_spec->key.id, if_index);

    // Convert VRF ID to name
    vrf_name = std::to_string (subnet_spec->vpc.id);

    req.set_entityindex (PDSA_LIM_ENT_INDEX);
    req.set_ifindex (if_index);
    req.set_ifenable (AMB_TRUE);
    req.set_ipv4enabled (AMB_TRISTATE_TRUE);
    req.set_ipv4fwding (AMB_TRISTATE_TRUE);
    req.set_ipv6enabled (AMB_TRISTATE_TRUE);
    req.set_ipv6fwding (AMB_TRISTATE_TRUE);
    req.set_fwdingmode (AMB_LIM_FWD_MODE_L3);
    req.set_vrfnamelen (vrf_name.length());
    req.set_vrfname (vrf_name);
}

static void
populate_evpn_if_bing_cfg_spec (pds_subnet_spec_t        *subnet_spec,
                                pds::EvpnIfBindCfgSpec&  req, 
                                uint32_t                 if_index)
{
    req.set_entityindex (PDSA_EVPN_ENT_INDEX);
    req.set_ifindex (if_index);
    req.set_eviindex (subnet_spec->key.id);
}


static void
populate_lim_swif_cfg_spec (pds::LimInterfaceCfgSpec& req,
                            uint32_t                  if_index)
{
    req.set_entityindex (PDSA_LIM_ENT_INDEX);
    req.set_ifindex (if_index);
    req.set_ifenable (AMB_TRUE);
    req.set_ipv4enabled (AMB_TRISTATE_FALSE);
    req.set_ipv4fwding (AMB_TRISTATE_FALSE);
    req.set_ipv6enabled (AMB_TRISTATE_FALSE);
    req.set_ipv6fwding (AMB_TRISTATE_FALSE);
    req.set_fwdingmode (AMB_LIM_FWD_MODE_L2);
    req.set_vrfnamelen (0);
}

static void
populate_lim_soft_if_spec (pds::LimSoftwIfSpec& req,
                           pds_ifindex_t        host_ifindex)
{
    req.set_entityindex (PDSA_LIM_ENT_INDEX);
    req.set_ifindex (LIF_IFINDEX_TO_LIF_ID(host_ifindex));
    req.set_iftype (AMB_LIM_SOFTWIF_DUMMY);
}
static void
process_subnet_update (pds_subnet_spec_t   *subnet_spec,
                       NBB_LONG            row_status)
{
    uint32_t lif_ifindex;
    
    PDSA_START_TXN(PDSA_CTM_GRPC_CORRELATOR);

    // Validate LIF
    SDK_ASSERT((subnet_spec->host_ifindex != IFINDEX_INVALID));

    // EVPN BD Row Update
    pds::EvpnBdSpec evpn_bd_spec;
    populate_evpn_bd_spec (subnet_spec, evpn_bd_spec);
    pdsa_set_amb_evpn_bd (evpn_bd_spec, row_status, PDSA_CTM_GRPC_CORRELATOR);

    // limGenIrbInterfaceTable Row Update
    pds::LimGenIrbIfSpec irb_spec;
    populate_lim_irb_spec (subnet_spec, irb_spec);
    pdsa_set_amb_lim_gen_irb_if (irb_spec, row_status, PDSA_CTM_GRPC_CORRELATOR);

    // Update IRB to VRF binding
    pds::LimInterfaceCfgSpec lim_if_spec;
    populate_lim_irb_if_cfg_spec (subnet_spec, lim_if_spec);
    pdsa_set_amb_lim_if_cfg (lim_if_spec, row_status, PDSA_CTM_GRPC_CORRELATOR);

    if (subnet_spec->host_ifindex != IFINDEX_INVALID) {
        // Create Lif here for now
        pds::LimSoftwIfSpec lim_swif_spec;
        populate_lim_soft_if_spec (lim_swif_spec, subnet_spec->host_ifindex);
        pdsa_set_amb_lim_software_if (lim_swif_spec, row_status, PDSA_CTM_GRPC_CORRELATOR);

        // Get Lif's MS IfIndex
        lif_ifindex = pds_to_ms_ifindex (subnet_spec->host_ifindex, IF_TYPE_LIF);
        SDK_TRACE_DEBUG ("SW Interface:: PDS IfIndex: 0x%X MSIfIndex: 0x%X",
                         subnet_spec->host_ifindex, lif_ifindex);

        // Set Lif interface settings
        pds::LimInterfaceCfgSpec lim_swifcfg_spec;
        populate_lim_swif_cfg_spec (lim_swifcfg_spec, lif_ifindex);
        pdsa_set_amb_lim_if_cfg (lim_swifcfg_spec, row_status, PDSA_CTM_GRPC_CORRELATOR);

        // evpnIfBindCfgTable Row Update
        pds::EvpnIfBindCfgSpec evpn_if_bind_spec;
        populate_evpn_if_bing_cfg_spec (subnet_spec, evpn_if_bind_spec, lif_ifindex);
        pdsa_set_amb_evpn_if_bind_cfg (evpn_if_bind_spec, row_status, PDSA_CTM_GRPC_CORRELATOR);
    }

    // TODO: Configure IRB IP Address??
    // TODO AMB_EVPN_EVI_RT for manual RT

    PDSA_END_TXN(PDSA_CTM_GRPC_CORRELATOR);

    // blocking on response from MS
    pds_ms::mgmt_state_t::ms_response_wait();

    // TODO: read return status from ms_response and send to caller
   
    return ;
}

struct subnet_upd_flags_t {
    bool bd = false;
    bool bd_if = false;
    bool irb = false;
    operator bool() {
        return (bd || bd_if || irb);
    }
};

static void
process_subnet_field_update (pds_subnet_spec_t   *subnet_spec,
                             const subnet_upd_flags_t& ms_upd_flags,
                             NBB_LONG             row_status)
{
    uint32_t lif_ifindex;

    PDSA_START_TXN(PDSA_CTM_GRPC_CORRELATOR);

    // EVPN BD Row Update
    if (ms_upd_flags.bd) {
        SDK_TRACE_DEBUG("Subnet %d Update: Trigger MS BD Update", subnet_spec->key.id);  
        pds::EvpnBdSpec evpn_bd_spec;
        populate_evpn_bd_spec (subnet_spec, evpn_bd_spec);
        pdsa_set_amb_evpn_bd (evpn_bd_spec, row_status, PDSA_CTM_GRPC_CORRELATOR);
    }

    // Create Lif here for now
    if (ms_upd_flags.bd_if) {
        SDK_TRACE_DEBUG("Subnet %d Update: Trigger MS BD If Update", subnet_spec->key.id);  
        pds::LimSoftwIfSpec lim_swif_spec;
        populate_lim_soft_if_spec (lim_swif_spec, subnet_spec->host_ifindex);
        pdsa_set_amb_lim_software_if (lim_swif_spec, row_status, PDSA_CTM_GRPC_CORRELATOR);

        // Get Lif's MS IfIndex
        lif_ifindex = pds_to_ms_ifindex (subnet_spec->host_ifindex, IF_TYPE_LIF);
        SDK_TRACE_DEBUG ("SW Interface:: PDS IfIndex: 0x%X MSIfIndex: 0x%X",
                         subnet_spec->host_ifindex, lif_ifindex);

        // Set Lif interface settings
        pds::LimInterfaceCfgSpec lim_swifcfg_spec;
        populate_lim_swif_cfg_spec (lim_swifcfg_spec, lif_ifindex);
        pdsa_set_amb_lim_if_cfg (lim_swifcfg_spec, row_status, PDSA_CTM_GRPC_CORRELATOR);

        // evpnIfBindCfgTable Row Update
        pds::EvpnIfBindCfgSpec evpn_if_bind_spec;
        populate_evpn_if_bing_cfg_spec (subnet_spec, evpn_if_bind_spec, lif_ifindex);
        pdsa_set_amb_evpn_if_bind_cfg (evpn_if_bind_spec, row_status, PDSA_CTM_GRPC_CORRELATOR);
    }

    if (ms_upd_flags.irb) {
        SDK_TRACE_DEBUG("Subnet %d Update: Trigger MS IRB Update", subnet_spec->key.id);  
        // TODO: Configure IRB IP Address??
        // TODO AMB_EVPN_EVI_RT for manual RT
    }

    PDSA_END_TXN(PDSA_CTM_GRPC_CORRELATOR);

    // blocking on response from MS
    pds_ms::mgmt_state_t::ms_response_wait();

    // TODO: read return status from ms_response and send to caller
   
    return ;
}

using pdsa_stub::bd_obj_t;
using pdsa_stub::bd_obj_uptr_t;
using pdsa_stub::state_t;

static void 
cache_subnet_spec(pds_subnet_spec_t* spec, bool op_delete) 
{
    auto state_ctxt = state_t::thread_context();
    if (op_delete) {
        state_ctxt.state()->bd_store().erase(spec->key.id);
        return;
    }
    bd_obj_uptr_t bd_obj_uptr (new bd_obj_t(*spec));
    state_ctxt.state()->bd_store().add_upd(spec->key.id, std::move(bd_obj_uptr));
}

sdk_ret_t
subnet_create (pds_subnet_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    cache_subnet_spec (spec, false /* Create new*/);
    process_subnet_update (spec, AMB_ROW_ACTIVE);

    // TODO: Get correct return code from CTM callback
    return SDK_RET_OK;
}

sdk_ret_t
subnet_delete (pds_subnet_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    process_subnet_update (spec, AMB_ROW_DESTROY);
    cache_subnet_spec (spec, true /* Delete */);

    // TODO: Get correct return code from CTM callback
    return SDK_RET_OK;
}

sdk_ret_t
subnet_update (pds_subnet_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    subnet_upd_flags_t  ms_upd_flags;
    bool fastpath = false;

    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = state_t::thread_context();
        auto bd_obj = state_ctxt.state()->bd_store().get(spec->key.id);
        if (bd_obj == nullptr) {
            SDK_TRACE_ERR("Update for unknown subnet %d", spec->key.id);
            return SDK_RET_ENTRY_NOT_FOUND;
        }

        auto& bd_pds_spec = bd_obj->properties().pds_spec;
        if (memcmp (&bd_pds_spec.fabric_encap, &spec->fabric_encap, 
                    sizeof(bd_pds_spec.fabric_encap)) != 0) {
            ms_upd_flags.bd = true;
            SDK_TRACE_INFO("Subnet %d VNI change - Old %d New %d", spec->key.id,
                           bd_pds_spec.fabric_encap.val.vnid,
                           spec->fabric_encap.val.vnid);
            bd_obj->properties().pds_spec.fabric_encap = spec->fabric_encap;
        }
        if (bd_pds_spec.host_ifindex != spec->host_ifindex) {
            ms_upd_flags.bd_if = true;
            SDK_TRACE_INFO("Subnet %d Host If change - Old 0x%x New 0x%x",
                           spec->key.id, bd_pds_spec.host_ifindex, 
                           spec->host_ifindex);
            bd_pds_spec.host_ifindex = spec->host_ifindex;
        }
        if (bd_pds_spec.v4_vr_ip != spec->v4_vr_ip) {
            ms_upd_flags.irb = true;
        }
        // Diff in any other property needs to be driven through fastpath
        if (memcmp(&bd_pds_spec, spec, sizeof(*spec)) != 0) {
            SDK_TRACE_INFO("Subnet %d fastpath parameter change", spec->key.id);
            fastpath = true;
        }
        // Update the cached subnet spec with the new info
        bd_obj->properties().pds_spec = *spec;

        if (fastpath) {
            // Stub takes care of sequencing if create has not yet been
            // received from MS.
            auto ret = l2f_bd_update_pds_synch(std::move(state_ctxt), bd_obj);
            // Do not state_ctxt has been released above 
            // Do not access global state beyond this
            if (ret != SDK_RET_OK) {
                SDK_TRACE_ERR("Subnet update fastpath fields failed %d", ret);
                return ret;
            }
        }
    } // End of state thread_context
      // Do Not access/modify global state after this

    if (ms_upd_flags) {
        process_subnet_field_update(spec, ms_upd_flags, AMB_ROW_ACTIVE);
        // TODO: Get correct return code from CTM callback
    }
    return SDK_RET_OK;
}
};    // namespace pds_ms

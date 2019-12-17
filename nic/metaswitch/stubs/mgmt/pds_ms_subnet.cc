// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_subnet.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_ctm.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/mgmt/pdsa_internal_utils_gen.hpp"
#include "gen/proto/internal.pb.h"
#include "nic/metaswitch/stubs/common/pds_ms_ifindex.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/sdk/include/sdk/if.hpp"

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

    // TODO: Configure IRB IP Address??
    // TODO AMB_EVPN_EVI_RT for manual RT

    PDSA_END_TXN(PDSA_CTM_GRPC_CORRELATOR);

    // blocking on response from MS
    pds_ms::mgmt_state_t::ms_response_wait();

    // TODO: read return status from ms_response and send to caller
   
    return ;
}

sdk_ret_t
subnet_create (pds_subnet_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    process_subnet_update (spec, AMB_ROW_ACTIVE);

    // TODO: Get correct return code from CTM callback
    return SDK_RET_OK;
}

sdk_ret_t
subnet_delete (pds_subnet_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    process_subnet_update (spec, AMB_ROW_DESTROY);

    // TODO: Get correct return code from CTM callback
    return SDK_RET_OK;
}

sdk_ret_t
subnet_update (pds_subnet_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    // TBD

    // TODO: Get correct return code from CTM callback
    return SDK_RET_OK;
}
};    // namespace pds_ms

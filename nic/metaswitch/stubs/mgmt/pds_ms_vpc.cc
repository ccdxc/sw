// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
#include "nic/metaswitch/stubs/mgmt/pds_ms_vpc.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_ctm.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/mgmt/pdsa_internal_utils_gen.hpp"
#include "nic/metaswitch/stubs/common/pdsa_vpc_store.hpp"
#include "nic/metaswitch/stubs/common/pdsa_state.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"

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
populate_evpn_ip_vrf_spec (pds_vpc_spec_t        *vpc_spec,
                           pds::EvpnIpVrfSpec&   req)
{
    std::string vrf_name;
    
    // Convert VRF ID to name
    vrf_name = std::to_string (vpc_spec->key.id);

    req.set_entityindex (PDSA_LIM_ENT_INDEX); 
    req.set_vrfname (vrf_name);
    req.set_vrfnamelen (vrf_name.length());
    req.set_vni (vpc_spec->fabric_encap.val.vnid);
    req.set_defaultrd (AMB_TRUE);
}

static void
pds_cache_vni_to_vrf_mapping (pds_vpc_spec_t *vpc_spec, bool op_delete)
{
    auto state_ctxt = pdsa_stub::state_t::thread_context();

    if (!op_delete) {
        auto vpc_obj = 
            (new pdsa_stub::vpc_obj_t(*vpc_spec));

        state_ctxt.state()->vpc_store().add_upd(vpc_spec->key.id, vpc_obj);
    } 
        state_ctxt.state()->vpc_store().erase(vpc_spec->key.id);}

static void
process_vpc_update (pds_vpc_spec_t *vpc_spec, 
                    NBB_LONG       row_status)
{
    PDSA_GET_SHARED_START ();
    NBB_TRC_ENTRY ("process_vpc_update");

    // Start CTM transaction
    pdsa_ctm_send_transaction_start (PDSA_CTM_GRPC_CORRELATOR);

    // LIM VRF Row Update
    pds::LimVrfSpec lim_vrf_spec;
    populate_lim_vrf_spec (vpc_spec, lim_vrf_spec);
    pdsa_set_amb_lim_vrf (lim_vrf_spec, row_status, PDSA_CTM_GRPC_CORRELATOR);

    // EVPN IP VRF Row Update
    pds::EvpnIpVrfSpec evpn_ip_vrf_spec;
    populate_evpn_ip_vrf_spec (vpc_spec, evpn_ip_vrf_spec);
    pdsa_set_amb_evpn_ip_vrf (evpn_ip_vrf_spec, row_status, PDSA_CTM_GRPC_CORRELATOR);

    // TODO: AMB_EVPN_IP_VRF_RT to configure manual RT

    //End CTM transaction
    pdsa_ctm_send_transaction_end (PDSA_CTM_GRPC_CORRELATOR);
   
    NBB_TRC_EXIT();
    PDSA_GET_SHARED_END();

    // blocking on response from MS
    pds_ms::mgmt_state_t::ms_response_wait();
   
    // TODO: read return status from ms_response and send to caller
    return ;
}

sdk_ret_t
vpc_create (pds_vpc_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    // cache VPC spec to be used in hals stub
    pds_cache_vni_to_vrf_mapping (spec, FALSE);
    process_vpc_update (spec, AMB_ROW_ACTIVE);

    // TODO: Get correct return code from CTM callback
    return SDK_RET_OK;
}

sdk_ret_t
vpc_delete (pds_vpc_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    process_vpc_update (spec, AMB_ROW_DESTROY);

    // Remove cached VPS spec, after successful reply from MS
    pds_cache_vni_to_vrf_mapping (spec, TRUE);

    // TODO: Get correct return code from CTM callback
    return SDK_RET_OK;
}

sdk_ret_t
vpc_update (pds_vpc_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    // TODO

    // TODO: Get correct return code from CTM callback
    return SDK_RET_OK;
}

};    // namespace pds_ms

// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
#include "nic/metaswitch/stubs/mgmt/pds_ms_subnet.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_ctm.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_lim_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_evpn_utils.hpp"

namespace pds_ms {
static void
process_subnet_update (pds_subnet_spec_t   *spec,
                       NBB_LONG            row_status)
{
    NBB_TRC_ENTRY ("process_subnet_update");

    // Start CTM transaction
    pdsa_ctm_send_transaction_start (PDSA_CTM_CORRELATOR);

    // EVPN EVI Row Update
    pdsa_stub::pdsa_row_update_evpn_evi (spec->key.id,
                              spec->fabric_encap.type,
                              row_status, 
                              PDSA_CTM_CORRELATOR);

    // EVPN BD Row Update
    pdsa_stub::pdsa_row_update_evpn_bd (spec->key.id, 
                             spec->fabric_encap.val.vnid,
                             row_status, 
                             PDSA_CTM_CORRELATOR);

    // limGenIrbInterfaceTable Row Update
    pdsa_stub::pdsa_row_update_lim_gen_irb_if (spec->key.id,
                                    row_status,
                                    PDSA_CTM_CORRELATOR);

    // TODO: Update IRB to VRF binding

    //End CTM transaction
    pdsa_ctm_send_transaction_end (PDSA_CTM_CORRELATOR);
   
    NBB_TRC_EXIT();
    return ;
}

static void
process_subnet_vnic_update (pds_vnic_spec_t *vnic_spec,
                            NBB_LONG        row_status)
{
    NBB_TRC_ENTRY ("process_subnet_vnic_update");

    // Start CTM transaction
    pdsa_ctm_send_transaction_start (PDSA_CTM_CORRELATOR);

    // evpnIfBindCfgTable Row Update
    pdsa_stub::pdsa_row_update_evpn_if_bind_cfg (vnic_spec->subnet.id,
                                      vnic_spec->key.id, 
                                      row_status,
                                      PDSA_CTM_CORRELATOR);

    //End CTM transaction
    pdsa_ctm_send_transaction_end (PDSA_CTM_CORRELATOR);
   
    NBB_TRC_EXIT();
    return ;
}

void
subnet_vnic_bind (pds_vnic_spec_t *vnic_spec)
{
    NBB_TRC_ENTRY ("subnet_vnic_bind");

    process_subnet_vnic_update (vnic_spec, AMB_ROW_ACTIVE);

    NBB_TRC_EXIT();
    return;
}

void
subnet_vnic_unbind (pds_vnic_spec_t *vnic_spec)
{
    NBB_TRC_ENTRY ("subnet_vnic_unbind");

    process_subnet_vnic_update (vnic_spec, AMB_ROW_DESTROY);

    NBB_TRC_EXIT();
    return;
}

sdk_ret_t
subnet_create (pds_subnet_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    NBB_TRC_ENTRY ("subnet_create");
    
    process_subnet_update (spec, AMB_ROW_ACTIVE);

    NBB_TRC_EXIT();

    // TODO: Get correct return code from CTM callback
    return SDK_RET_OK;
}

sdk_ret_t
subnet_delete (pds_subnet_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    NBB_TRC_ENTRY ("subnet_delete");
    
    process_subnet_update (spec, AMB_ROW_DESTROY);

    NBB_TRC_EXIT();

    // TODO: Get correct return code from CTM callback
    return SDK_RET_OK;
}

sdk_ret_t
subnet_update (pds_subnet_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    NBB_TRC_ENTRY ("subnet_update");
    
    // TBD

    NBB_TRC_EXIT();

    // TODO: Get correct return code from CTM callback
    return SDK_RET_OK;
}

};    // namespace pds_ms

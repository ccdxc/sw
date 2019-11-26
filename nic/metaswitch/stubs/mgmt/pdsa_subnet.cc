// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
#include "nic/metaswitch/stubs/mgmt/pdsa_subnet.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_ctm.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_lim_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_evpn_utils.hpp"

namespace pdsa_stub {
static void
pdsa_subnet_update (pds_subnet_spec_t   *subnet_spec,
                    NBB_LONG            row_status)
{
    NBB_TRC_ENTRY ("pdsa_subnet_update");

    // Start CTM transaction
    pdsa_ctm_send_transaction_start (PDSA_CTM_CORRELATOR);

    // EVPN EVI Row Update
    pdsa_row_update_evpn_evi (subnet_spec->key.id,
                              subnet_spec->fabric_encap.type,
                              row_status, 
                              PDSA_CTM_CORRELATOR);

    // EVPN BD Row Update
    pdsa_row_update_evpn_bd (subnet_spec->key.id, 
                             subnet_spec->fabric_encap.val.vnid,
                             row_status, 
                             PDSA_CTM_CORRELATOR);

    // limGenIrbInterfaceTable Row Update
    pdsa_row_update_lim_gen_irb_if (subnet_spec->key.id,
                                    row_status,
                                    PDSA_CTM_CORRELATOR);

    // TODO: Update IRB to VRF binding

    //End CTM transaction
    pdsa_ctm_send_transaction_end (PDSA_CTM_CORRELATOR);
   
    NBB_TRC_EXIT();
    return ;
}

static void
pdsa_subnet_vnic_update (pds_vnic_spec_t *vnic_spec,
                         NBB_LONG        row_status)
{
    NBB_TRC_ENTRY ("pdsa_subnet_vnic_update");

    // Start CTM transaction
    pdsa_ctm_send_transaction_start (PDSA_CTM_CORRELATOR);

    // evpnIfBindCfgTable Row Update
    pdsa_row_update_evpn_if_bind_cfg (vnic_spec->subnet.id,
                                      vnic_spec->key.id, 
                                      row_status,
                                      PDSA_CTM_CORRELATOR);

    //End CTM transaction
    pdsa_ctm_send_transaction_end (PDSA_CTM_CORRELATOR);
   
    NBB_TRC_EXIT();
    return ;
}

void 
pdsa_subnet_create (pds_subnet_spec_t *subnet_spec)
{
    NBB_TRC_ENTRY ("pdsa_subnet_create");
    
    pdsa_subnet_update (subnet_spec, AMB_ROW_ACTIVE);

    NBB_TRC_EXIT();
    return;
}

void
pdsa_subnet_delete (pds_subnet_spec_t *subnet_spec)
{
    NBB_TRC_ENTRY ("pdsa_subnet_delete");
    
    pdsa_subnet_update (subnet_spec, AMB_ROW_DESTROY);

    NBB_TRC_EXIT();
    return;
}

void
pdsa_subnet_vnic_bind (pds_vnic_spec_t *vnic_spec)
{
    NBB_TRC_ENTRY ("pdsa_subnet_vnic_bind");

    pdsa_subnet_vnic_update (vnic_spec, AMB_ROW_ACTIVE);

    NBB_TRC_EXIT();
    return;
}

void
pdsa_subnet_vnic_unbind (pds_vnic_spec_t *vnic_spec)
{
    NBB_TRC_ENTRY ("pdsa_subnet_vnic_unbind");

    pdsa_subnet_vnic_update (vnic_spec, AMB_ROW_DESTROY);

    NBB_TRC_EXIT();
    return;
}
};

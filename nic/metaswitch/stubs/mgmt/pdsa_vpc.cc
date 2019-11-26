// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
#include "nic/metaswitch/stubs/mgmt/pdsa_vpc.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_ctm.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_evpn_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_lim_utils.hpp"

namespace pdsa_stub {

static void
pdsa_vpc_update (pds_vpc_spec_t *vpc_spec, 
                 NBB_LONG       row_status)
{
    NBB_TRC_ENTRY ("pdsa_vpc_update");

    // Start CTM transaction
    pdsa_ctm_send_transaction_start (PDSA_CTM_CORRELATOR);

    // LIM VRF Row Update
    pdsa_row_update_lim_vrf (vpc_spec->key.id, 
                             row_status, 
                             PDSA_CTM_CORRELATOR);

    // EVPN IP VRF Row Update
    pdsa_row_update_evpn_ip_vrf (vpc_spec->key.id,
                                 vpc_spec->fabric_encap.val.vnid,
                                 row_status, 
                                 PDSA_CTM_CORRELATOR);

    // TODO: Store VNI to VRF mapping                                 

    //End CTM transaction
    pdsa_ctm_send_transaction_end (PDSA_CTM_CORRELATOR);
   
    NBB_TRC_EXIT();
    return ;
}

void
pdsa_vpc_create (pds_vpc_spec_t *vpc_spec)
{
    NBB_TRC_ENTRY ("pdsa_vpc_create");
    
    pdsa_vpc_update (vpc_spec, AMB_ROW_ACTIVE);

    NBB_TRC_EXIT();
    return;
}

void
pdsa_vpc_delete (pds_vpc_spec_t *vpc_spec)
{
    NBB_TRC_ENTRY ("pdsa_vpc_delete");
    
    pdsa_vpc_update (vpc_spec, AMB_ROW_DESTROY);

    NBB_TRC_EXIT();
    return;
}

};

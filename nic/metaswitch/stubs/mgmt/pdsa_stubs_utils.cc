//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//---------------------------------------------------------------
#include "nic/metaswitch/stubs/mgmt/pdsa_stubs_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_utils.hpp"

namespace pdsa_stub {
extern NBB_VOID
pdsa_li_stub_create (pdsa_config_t& conf);

void pdsa_stubs_create ()
{
    // Local variables
    pdsa_config_t   conf = {0};
    
    /***************************************************************************/
    /* Get the lock for the SHARED LOCAL data.                                 */
    /***************************************************************************/
    NBS_ENTER_SHARED_CONTEXT(sms_our_pid);
    NBS_GET_SHARED_DATA();

    NBB_TRC_FLOW ((NBB_FORMAT "Start CTM Transaction"));
    pdsa_ctm_send_transaction_start (PDSA_CTM_STUB_INIT_CORRELATOR);

    NBB_TRC_FLOW ((NBB_FORMAT "ROW UPDATES to initialize Stubs and Components"));

    conf.correlator  = PDSA_CTM_STUB_INIT_CORRELATOR;
    conf.row_status  = AMB_ROW_ACTIVE;

    pdsa_l2f_stub_create (&conf);   // L2F stub
    pdsa_li_stub_create (&conf);    // LI stub
    pdsa_lim_create (&conf);        // LI Mgr
    pdsa_smi_stub_create (&conf);   // SMI stub
    pdsa_sck_stub_create (&conf);   // Sck stub
    pdsa_ftm_create (&conf);        // FT Mgr
    pdsa_hals_stub_create (&conf);  // HALS Stub
    pdsa_nar_stub_create (&conf);   // NAR stub
    pdsa_nrm_create (&conf);        // NR Mgr
    pdsa_psm_create (&conf);        // PS Mgr
    pdsa_ft_stub_create (&conf);    // FT stub
    pdsa_rtm_create (&conf);        // RT Mgr
    pdsa_bgp_create (&conf);        // BGP component
    pdsa_evpn_create (&conf);       // EVPN component

    NBB_TRC_FLOW ((NBB_FORMAT "End CTM Transaction"));
    pdsa_ctm_send_transaction_end (PDSA_CTM_STUB_INIT_CORRELATOR);

    /***************************************************************************/ 
    /* Release the lock on the SHARED LOCAL data.                              */
    /***************************************************************************/
    NBS_RELEASE_SHARED_DATA();
    NBS_EXIT_SHARED_CONTEXT();
}

}

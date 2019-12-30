//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//---------------------------------------------------------------
#include "nic/metaswitch/stubs/mgmt/pds_ms_stubs_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"

namespace pds_ms {
extern NBB_VOID
pds_ms_li_stub_create (pds_ms_config_t& conf);

void pds_mss_create ()
{
    // Local variables
    pds_ms_config_t   conf = {0};
    
    /***************************************************************************/
    /* Get the lock for the SHARED LOCAL data.                                 */
    /***************************************************************************/
    NBS_ENTER_SHARED_CONTEXT(sms_our_pid);
    NBS_GET_SHARED_DATA();

    NBB_TRC_FLOW ((NBB_FORMAT "Start CTM Transaction"));
    pds_ms_ctm_send_transaction_start (PDS_MS_CTM_STUB_INIT_CORRELATOR);

    NBB_TRC_FLOW ((NBB_FORMAT "ROW UPDATES to initialize Stubs and Components"));

    conf.correlator  = PDS_MS_CTM_STUB_INIT_CORRELATOR;
    conf.row_status  = AMB_ROW_ACTIVE;

    pds_ms_l2f_stub_create (&conf);   // L2F stub
    pds_ms_li_stub_create (&conf);    // LI stub
    pds_ms_lim_create (&conf);        // LI Mgr
    pds_ms_smi_stub_create (&conf);   // SMI stub
    pds_ms_sck_stub_create (&conf);   // Sck stub
    pds_ms_ftm_create (&conf);        // FT Mgr
    pds_ms_hals_stub_create (&conf);  // HALS Stub
    pds_ms_nar_stub_create (&conf);   // NAR stub
    pds_ms_nrm_create (&conf);        // NR Mgr
    pds_ms_psm_create (&conf);        // PS Mgr
    pds_ms_ft_stub_create (&conf);    // FT stub
    pds_ms_rtm_create (&conf);        // RT Mgr
    pds_ms_bgp_create (&conf);        // BGP component
    pds_ms_evpn_create (&conf);       // EVPN component

    NBB_TRC_FLOW ((NBB_FORMAT "End CTM Transaction"));
    pds_ms_ctm_send_transaction_end (PDS_MS_CTM_STUB_INIT_CORRELATOR);

    /***************************************************************************/ 
    /* Release the lock on the SHARED LOCAL data.                              */
    /***************************************************************************/
    NBS_RELEASE_SHARED_DATA();
    NBS_EXIT_SHARED_CONTEXT();
}

}

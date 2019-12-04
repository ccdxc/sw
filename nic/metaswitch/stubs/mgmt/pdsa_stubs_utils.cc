//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//---------------------------------------------------------------
#include "nic/metaswitch/stubs/mgmt/pdsa_stubs_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_utils.hpp"

namespace pdsa_stub {

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

    NBB_TRC_FLOW ((NBB_FORMAT "ROW UPDATES to initialize Stubs and MS Processes"));

    conf.correlator         = PDSA_CTM_STUB_INIT_CORRELATOR;
    conf.row_status         = AMB_ROW_ACTIVE;

    // l2fEntTable
    pdsa_row_update_l2f (&conf);

    // liEntTable
    pdsa_row_update_li (&conf);

    // smiEntTable
    pdsa_row_update_smi (&conf);

    // liMjTable
    pdsa_row_update_li_mj (&conf, 
                           AMB_LI_IF_ATG_FRI,
                           AMB_LI_MJ_PARTNER_LIM,
                           1,
                           0);

    // limEntTable
    pdsa_row_update_lim (&conf);

    // limMjTable - LIPI
    pdsa_row_update_lim_mj (&conf,
                            AMB_LIM_IF_ATG_LIPI,
                            AMB_LIM_MJ_PARTNER_LI,
                            1,
                            0);

    // limMjTable - SMI
    pdsa_row_update_lim_mj (&conf,
                            AMB_LIM_IF_ATG_SMI,
                            AMB_LIM_MJ_PARTNER_SMI,
                            1,
                            0);

    // sckTable 
    pdsa_row_update_sck (&conf);

    // ftmEntTable
    pdsa_row_update_ftm (&conf);

    // ftmMjTable - ROPI
    pdsa_row_update_ftm_mj (&conf,
                            AMB_FTM_IF_ATG_ROPI,
                            AMB_FTM_MJ_PARTNER_HALS,
                            1,
                            0);

    // ftmMjTable - PRI
    pdsa_row_update_ftm_mj (&conf,
                            AMB_FTM_IF_ATG_PRI,
                            AMB_FTM_MJ_PARTNER_PSM,
                            1,
                            0);

    // halsEntTable
    pdsa_row_update_hals (&conf);

    // narEntTable
    pdsa_row_update_nar (&conf);

    // nrmEntTable
    pdsa_row_update_nrm (&conf);

    // nrmMjTable - AMB_NRM_IF_ATG_NARI
    pdsa_row_update_nrm_mj (&conf,
                            AMB_NRM_IF_ATG_NARI,
                            AMB_NRM_MJ_PARTNER_NAR,
                            1,
                            0);

    // nrmMjTable - AMB_NRM_IF_ATG_NBPI
    pdsa_row_update_nrm_mj (&conf,
                            AMB_NRM_IF_ATG_NBPI,
                            AMB_NRM_MJ_PARTNER_HALS,
                            1,
                            0);

    // nrmMjTable - AMB_NRM_IF_ATG_MMI 
    pdsa_row_update_nrm_mj (&conf,
                            AMB_NRM_IF_ATG_MMI,
                            AMB_NRM_MJ_PARTNER_L2FST,
                            1,
                            0);

    // nrmMjTable - AMB_NRM_IF_ATG_I3 
    pdsa_row_update_nrm_mj (&conf,
                            AMB_NRM_IF_ATG_I3,
                            AMB_NRM_MJ_PARTNER_LIM,
                            1,
                            0);

    // psmEntTable
    pdsa_row_update_psm (&conf);

    // psmMjTable - AMB_PSM_IF_ATG_NHPI
    pdsa_row_update_psm_mj (&conf,
                            AMB_PSM_IF_ATG_NHPI,
                            AMB_PSM_MJ_PARTNER_HALS,
                            1,
                            0);

    // psmMjTable - AMB_PSM_IF_ATG_NRI
    pdsa_row_update_psm_mj (&conf,
                            AMB_PSM_IF_ATG_NRI,
                            AMB_PSM_MJ_PARTNER_NRM,
                            1,
                            0);

    // psmMjTable - AMB_PSM_IF_ATG_NHPI
    pdsa_row_update_psm_mj (&conf,
                            AMB_PSM_IF_ATG_I3,
                            AMB_PSM_MJ_PARTNER_LIM,
                            1,
                            0);

    // FT Stub - ftsEntityTable
    pdsa_row_update_fts (&conf, AMB_ADMIN_STATUS_UP);

    // rtmEntityTable - Admin Down                                
    pdsa_row_update_rtm (&conf, AMB_ADMIN_STATUS_DOWN);

    // rtmMjTable - AMB_RTM_ARI_PARTNER_BGP
    pdsa_row_update_rtm_mj (&conf, AMB_RTM_ARI_PARTNER_BGP);

    // rtmMjTable -AMB_RTM_ARI_PARTNER_FT 
    pdsa_row_update_rtm_mj (&conf, AMB_RTM_ARI_PARTNER_FT);

    // rtmEntityTable - Admin UP                                
    pdsa_row_update_rtm (&conf, AMB_ADMIN_STATUS_UP);

    // rtm Redistribute connected
    pdsa_rtm_redis_connected (&conf);

    // bgpNmEntTable
    pdsa_row_update_bgp_nm (&conf);

    // evpnEntTable
    pdsa_row_update_evpn (&conf);

    // evpnMjTable - AMB_EVPN_IF_ATG_BDPI
    pdsa_row_update_evpn_mj (&conf,
                             AMB_EVPN_IF_ATG_BDPI,
                             AMB_EVPN_MJ_PARTNER_L2FST,
                             1,
                             0);

    // evpnMjTable - AMB_EVPN_IF_ATG_I3
    pdsa_row_update_evpn_mj (&conf,
                             AMB_EVPN_IF_ATG_I3,
                             AMB_EVPN_MJ_PARTNER_LIM,
                             1,
                             0);

    // evpnMjTable -AMB_EVPN_IF_ATG_TPI 
    pdsa_row_update_evpn_mj (&conf,
                             AMB_EVPN_IF_ATG_TPI,
                             AMB_EVPN_MJ_PARTNER_LIM,
                             1,
                             0);

    // evpnMjTable - AMB_EVPN_IF_ATG_MAI
    pdsa_row_update_evpn_mj (&conf,
                             AMB_EVPN_IF_ATG_MAI,
                             AMB_EVPN_MJ_PARTNER_L2FST,
                             1,
                             0);

    NBB_TRC_FLOW ((NBB_FORMAT "End CTM Transaction"));
    pdsa_ctm_send_transaction_end (PDSA_CTM_STUB_INIT_CORRELATOR);

    /***************************************************************************/ 
    /* Release the lock on the SHARED LOCAL data.                              */
    /***************************************************************************/
    NBS_RELEASE_SHARED_DATA();
    NBS_EXIT_SHARED_CONTEXT();
}

}

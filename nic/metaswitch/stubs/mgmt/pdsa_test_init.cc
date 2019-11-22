// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Temporary init sequence for metaswitch 

#include "nic/metaswitch/stubs/mgmt/pdsa_test_init.hpp"
#include "nic/metaswitch/stubs/common/pdsa_util.hpp"
#include "nic/metaswitch/stubs/pdsa_stubs_init.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_config.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_ifindex.hpp"

NBB_VOID
pdsa_test_init ()
{
    // Local variables
    NBB_ULONG       test_correlator = 0x100;
    pdsa_config_t   conf = {0};

    NBB_TRC_ENTRY ("pdsa_test_init");
    
    // TODO: Temporary init code to parse json config file
    if (parse_json_config(&conf) < 0) {
        fprintf(stderr, "parse_json_config error!\n");
        exit(1);
    }

    /***************************************************************************/
    /* Get the lock for the SHARED LOCAL data.                                 */
    /***************************************************************************/
    NBS_ENTER_SHARED_CONTEXT(sms_our_pid);
    NBS_GET_SHARED_DATA();

    NBB_TRC_FLOW ((NBB_FORMAT "Start CTM Transaction"));
    pdsa_ctm_send_transaction_start (test_correlator);

    NBB_TRC_FLOW ((NBB_FORMAT "ROW UPDATES"));

    // si
    conf.correlator         = test_correlator;
    //pdsa_test_row_update_si (&conf);

    // l2fEntTable
    pdsa_test_row_update_l2f (&conf);

    // liEntTable
    pdsa_test_row_update_li (&conf);

    //smiEntTable
    pdsa_test_row_update_smi (&conf);

    // liMjTable
    pdsa_test_row_update_li_mj (&conf,
                                AMB_LI_IF_ATG_FRI,
                                AMB_LI_MJ_PARTNER_LIM,
                                1,
                                0);

    // limEntTable
    pdsa_test_row_update_lim (&conf);

    // limMjTable - LIPI
    pdsa_test_row_update_lim_mj (&conf,
                                 AMB_LIM_IF_ATG_LIPI,
                                 AMB_LIM_MJ_PARTNER_LI,
                                 1,
                                 0);

    // limMjTable - SMI
    pdsa_test_row_update_lim_mj (&conf,
                                 AMB_LIM_IF_ATG_SMI,
                                 AMB_LIM_MJ_PARTNER_SMI,
                                 1,
                                 0);

    // limInterfaceCfgTable - L3 interface for physical port
    // NetAgent will provide the HAL L3 IfIndex. 
    // Mgmt Stub has to convert it to MS IfIndex
    auto ms_ifindex = pdsa_stub::pds_to_ms_ifindex(conf.g_evpn_if_index);
    SDK_TRACE_INFO ("Configuring uplink port %ld L3 IfIndex 0x%lx MS IfIndex 0x%lx", 
                    ETH_IFINDEX_TO_PARENT_PORT(conf.g_evpn_if_index), conf.g_evpn_if_index, ms_ifindex);
    pdsa_test_row_update_lim_if_cfg (&conf,
                                     ms_ifindex,
                                     AMB_TRUE,
                                     AMB_TRISTATE_TRUE,
                                     AMB_TRISTATE_TRUE,
                                     AMB_TRISTATE_TRUE,
                                     AMB_LIM_FWD_MODE_L3);
    // limL3InterfaceAddressTable
    pdsa_test_row_update_lim_if_addr (&conf);

    // limInterfaceCfgTable -NODE_A_AC_IF_INDEX 
    pdsa_test_row_update_lim_if_cfg (&conf,
                                     7,
                                     AMB_TRUE,
                                     AMB_TRISTATE_FALSE,
                                     AMB_TRISTATE_FALSE,
                                     AMB_TRISTATE_FALSE,
                                     AMB_LIM_FWD_MODE_DEFAULT);

    // sckTable 
    pdsa_test_row_update_sck (&conf);

    // l2fMacIpCfgTable
    pdsa_test_row_update_l2f_mac_ip_cfg(&conf);
    
    // ftmEntTable
    pdsa_test_row_update_ftm (&conf);

    // ftmMjTable - ROPI
    pdsa_test_row_update_ftm_mj (&conf,
                                 AMB_FTM_IF_ATG_ROPI,
                                 AMB_FTM_MJ_PARTNER_HALS,
                                 1,
                                 0);

    // ftmMjTable - PRI
    pdsa_test_row_update_ftm_mj (&conf,
                                 AMB_FTM_IF_ATG_PRI,
                                 AMB_FTM_MJ_PARTNER_PSM,
                                 1,
                                 0);

    // halsEntTable
    pdsa_test_row_update_hals (&conf);

    // narEntTable
    pdsa_test_row_update_nar (&conf);

    // nrmEntTable
    pdsa_test_row_update_nrm (&conf);

    // nrmMjTable - AMB_NRM_IF_ATG_NARI
    pdsa_test_row_update_nrm_mj (&conf,
                                 AMB_NRM_IF_ATG_NARI,
                                 AMB_NRM_MJ_PARTNER_NAR,
                                 1,
                                 0);

    // nrmMjTable - AMB_NRM_IF_ATG_NBPI
    pdsa_test_row_update_nrm_mj (&conf,
                                 AMB_NRM_IF_ATG_NBPI,
                                 AMB_NRM_MJ_PARTNER_HALS,
                                 1,
                                 0);

    // nrmMjTable - AMB_NRM_IF_ATG_MMI 
    pdsa_test_row_update_nrm_mj (&conf,
                                 AMB_NRM_IF_ATG_MMI,
                                 AMB_NRM_MJ_PARTNER_L2FST,
                                 1,
                                 0);

    // nrmMjTable - AMB_NRM_IF_ATG_I3 
    pdsa_test_row_update_nrm_mj (&conf,
                                 AMB_NRM_IF_ATG_I3,
                                 AMB_NRM_MJ_PARTNER_LIM,
                                 1,
                                 0);

    // psmEntTable
    pdsa_test_row_update_psm (&conf);

    // psmMjTable - AMB_PSM_IF_ATG_NHPI
    pdsa_test_row_update_psm_mj (&conf,
                                 AMB_PSM_IF_ATG_NHPI,
                                 AMB_PSM_MJ_PARTNER_HALS,
                                 1,
                                 0);

    // psmMjTable - AMB_PSM_IF_ATG_NRI
    pdsa_test_row_update_psm_mj (&conf,
                                 AMB_PSM_IF_ATG_NRI,
                                 AMB_PSM_MJ_PARTNER_NRM,
                                 1,
                                 0);

    // psmMjTable - AMB_PSM_IF_ATG_NHPI
    pdsa_test_row_update_psm_mj (&conf,
                                 AMB_PSM_IF_ATG_I3,
                                 AMB_PSM_MJ_PARTNER_LIM,
                                 1,
                                 0);

    // FT Stub - ftsEntityTable
    pdsa_test_row_update_fts (&conf, AMB_ADMIN_STATUS_UP);

    // rtmEntityTable - Admin Down                                
    pdsa_test_row_update_rtm (&conf, AMB_ADMIN_STATUS_DOWN);

    // rtmMjTable - AMB_RTM_ARI_PARTNER_BGP
    pdsa_test_row_update_rtm_mj (&conf, AMB_RTM_ARI_PARTNER_BGP);

    // rtmMjTable -AMB_RTM_ARI_PARTNER_FT 
    pdsa_test_row_update_rtm_mj (&conf, AMB_RTM_ARI_PARTNER_FT);

    // rtmEntityTable - Admin UP                                
    pdsa_test_row_update_rtm (&conf, AMB_ADMIN_STATUS_UP);

    // rtm Redistribute connected
    pdsa_rtm_redis_connected (&conf);

   // bgpRmEntTable
   pdsa_test_row_update_bgp_rm (&conf);

   // bgpNmEntTable
   pdsa_test_row_update_bgp_nm (&conf);

   // bgpRmAfiSafiTable
   pdsa_test_row_update_bgp_rm_afi_safi (&conf);

    // bgpNmListenTable
    pdsa_test_row_update_bgp_nm_listen (&conf);
    
    // bgpRmAfmJoinTable - AMB_BGP_AFI_IPV4
    pdsa_test_row_update_bgp_rm_afm_join (&conf,
                                          1, 
                                          AMB_BGP_AFI_IPV4, 
                                          AMB_BGP_UNICAST);

    // bgpRmAfmJoinTable - AMB_BGP_AFI_L2VPN
    pdsa_test_row_update_bgp_rm_afm_join (&conf,
                                          2, 
                                          AMB_BGP_AFI_L2VPN, 
                                          AMB_BGP_EVPN);

    // bgpPeerTable 
    pdsa_test_row_update_bgp_peer (&conf);

    // evpnEntTable
    pdsa_test_row_update_evpn (&conf);

    // evpnMjTable - AMB_EVPN_IF_ATG_BDPI
    pdsa_test_row_update_evpn_mj (&conf,
                                  AMB_EVPN_IF_ATG_BDPI,
                                  AMB_EVPN_MJ_PARTNER_L2FST,
                                  1,
                                  0);

    // evpnMjTable - AMB_EVPN_IF_ATG_I3
    pdsa_test_row_update_evpn_mj (&conf,
                                  AMB_EVPN_IF_ATG_I3,
                                  AMB_EVPN_MJ_PARTNER_LIM,
                                  1,
                                  0);

    // evpnMjTable -AMB_EVPN_IF_ATG_TPI 
    pdsa_test_row_update_evpn_mj (&conf,
                                  AMB_EVPN_IF_ATG_TPI,
                                  AMB_EVPN_MJ_PARTNER_LIM,
                                  1,
                                  0);

    // evpnMjTable - AMB_EVPN_IF_ATG_MAI
    pdsa_test_row_update_evpn_mj (&conf,
                                  AMB_EVPN_IF_ATG_MAI,
                                  AMB_EVPN_MJ_PARTNER_L2FST,
                                  1,
                                  0);

    // evpnEviTable
    pdsa_test_row_update_evpn_evi (&conf);

    // evpnBdTable
    pdsa_test_row_update_evpn_bd (&conf);

    // evpnIfBindCfgTable
    pdsa_test_row_update_evpn_if_bind_cfg (&conf);

    NBB_TRC_FLOW ((NBB_FORMAT "End CTM Transaction"));
    pdsa_ctm_send_transaction_end (test_correlator);

    /***************************************************************************/ 
    /* Release the lock on the SHARED LOCAL data.                              */
    /***************************************************************************/
    NBS_RELEASE_SHARED_DATA();
    NBS_EXIT_SHARED_CONTEXT();

    NBB_TRC_EXIT();
}

// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Initialize Mgmt Stub

#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_stubs_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_ifindex.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_hal_init.hpp"
#include "nic/apollo/agent/svc/service.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/svc/internal_bgp_gen.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/svc/internal_evpn_gen.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/svc/internal_cp_route_gen.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/svc/cp_test_gen.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/mgmt/pds_ms_internal_cp_route_utils_gen.hpp"
#include <nbase.h>
#include <nbbstub.h>
extern "C" {
#include <a0user.h>
#include <smsiincl.h>
}
#include <net/if.h>
#include <iostream>

using namespace std;
using namespace pds_ms;

extern NBB_BOOL sms_initialize(NBB_CXT_T NBB_CXT);
extern NBB_BOOL pds_ms_register_mib_notify_sink(NBB_CXT_T NBB_CXT);

static void
nbase_init ()
{
    NBB_BOOL nbase_init = FALSE;

    /***************************************************************************/
    /* Initialize the N-BASE.                                                  */
    /***************************************************************************/
    if (!NBS_INITIALIZE())
    {
      /*************************************************************************/
      /* FLOW TRACING NOT REQUIRED    Reason: Too early for trace.             */
      /*************************************************************************/
      NBB_PRINTF(" Failed to initialize N-BASE.\n");
      goto EXIT_LABEL;
    }

    /***************************************************************************/
    /* Enter N-BASE root context.  This allows this code to make N-BASE calls  */
    /* as it had been created as an N-BASE process.                            */
    /***************************************************************************/
    NBS_ENTER_SHARED_CONTEXT(NBS_ROOT_PROCESS);

    /***************************************************************************/
    /* Now that the N-BASE context has been set up we can start tracing.       */
    /***************************************************************************/
    NBB_TRC_ENTRY("smsi_main");

    /***************************************************************************/
    /* Initialize AMBL, AHL and AALL.                                          */
    /***************************************************************************/
    AMBL_INIT();
    AHL_INIT();
    AALL_INIT();

    /***************************************************************************/
    /* All the N-BASE processes that can be created in this instance of the    */
    /* N-BASE must be registered with the N-BASE.  Registration consists of    */
    /* supplying a create_proc (called immediately after the N-BASE process    */
    /* has been created, and a destroy_proc (called when the N-BASE process is */
    /* about to be destroyed).                                                 */
    /***************************************************************************/
    NBB_TRC_DETAIL((NBB_FORMAT "Registering processes..."));
    SMSI_INITIALIZE("DCSM", as0_initialize);
    SMSI_INITIALIZE(NULL, amb_initialize);
    SMSI_INITIALIZE("DCAI3", ai3_init);
    SMSI_INITIALIZE("DCAHSL", ahsl_init);
    SMSI_INITIALIZE("DCABDPL", abdpl_init);
    SMSI_INITIALIZE("DCQAQL", qaql_init);
    SMSI_INITIALIZE("SCK", sck_initialize);
//    SMSI_INITIALIZE("I3", i3_initialize);
//    SMSI_INITIALIZE("BD", bd_initialize);
    SMSI_INITIALIZE("MAL", mal_init);
    SMSI_INITIALIZE("DCAMH", amh_initialize); 
    SMSI_INITIALIZE("DCCSS", css_initialize);
//    SMSI_INITIALIZE("DCLMGR", rl0_initialize);
    SMSI_INITIALIZE("DCSMS", sms_initialize);


    SMSI_INITIALIZE("DCBGP", qb_initialize);
    SMSI_INITIALIZE("EVPN", evm_initialize);
    SMSI_INITIALIZE("DCQCRT", qcrt_initialize);
    SMSI_INITIALIZE("DCQCFT", qcft_initialize);
    SMSI_INITIALIZE("AMX", amx_initialize);
    SMSI_FTE_INITIALIZE();
    SMSI_LI_INITIALIZE();
    SMSI_LIM_INITIALIZE();
    SMSI_L2F_INITIALIZE();
    SMSI_NAR_INITIALIZE();
    SMSI_NRM_INITIALIZE();
    SMSI_PSM_INITIALIZE();
    SMSI_SMI_INITIALIZE();
    SMSI_HALS_INITIALIZE();
    SMSI_L2_INITIALIZE();
    SMSI_FTM_INITIALIZE();

    // Register user callback to convert between MS IfIndex & Linux IfIndex
    A0_USER_REG_LOCAL_IF_MAP_FN (ms_to_lnx_ifindex);
    A0_USER_REG_SYSTEM_IF_MAP_FN(lnx_to_ms_ifindex);

    pds_ms_reg_pre_set_amb_bgp_rm_ent (bgp_rm_ent_pre_set);
    pds_ms_reg_pre_get_amb_bgp_rm_ent (bgp_rm_ent_pre_get);
    pds_ms_reg_pre_set_amb_bgp_peer(bgp_peer_pre_set);
    pds_ms_reg_pre_get_amb_bgp_peer(bgp_peer_pre_get);
    pds_ms_reg_pre_set_amb_evpn_evi(evpn_evi_pre_set);
    pds_ms_reg_pre_get_amb_evpn_evi(evpn_evi_pre_get);
    pds_ms_reg_pre_set_amb_evpn_evi_rt(evpn_evi_rt_pre_set);
    pds_ms_reg_pre_get_amb_evpn_evi_rt(evpn_evi_rt_pre_get);
    pds_ms_reg_pre_set_amb_evpn_ip_vrf(evpn_ip_vrf_pre_set);
    pds_ms_reg_pre_get_amb_evpn_ip_vrf(evpn_ip_vrf_pre_get);
    pds_ms_reg_pre_set_amb_evpn_ip_vrf_rt(evpn_ip_vrf_rt_pre_set);
    pds_ms_reg_pre_get_amb_evpn_ip_vrf_rt(evpn_ip_vrf_rt_pre_get);
    pds_ms_reg_pre_set_amb_bgp_peer_afi_safi(bgp_peer_afi_safi_pre_set);
    pds_ms_reg_pre_get_amb_bgp_peer_afi_safi(bgp_peer_afi_safi_pre_get);
    pds_ms_reg_pre_fill_get_amb_bgp_peer_afi_safi(
        bgp_peer_afi_safi_pre_fill_get);
    pds_ms_reg_pre_set_amb_lim_l3_if_addr(lim_l3_if_addr_pre_set);
    pds_ms_reg_pre_set_amb_cipr_rtm_static_rt(cp_route_pre_set);

    /***************************************************************************/
    /* Initialize the System Manager create parms.                             */
    /*                                                                         */
    /* This is a primary instance of System Manager so the is_primary field    */
    /* should be set to ATG_YES.                                               */
    /*                                                                         */
    /* If the system supports multiple N-BASE locations then set               */
    /* location_capable to ATG_YES and fill in location_index,                 */
    /* location_group_index and hardware_location.                             */
    /*                                                                         */
    /* If the system supports fault tolerance then set ft_capable to ATG_YES.  */
    /*                                                                         */
    /* If the system supports fault tolerance then it must also support        */
    /* multiple N-BASE locations.                                              */
    /***************************************************************************/
    ASE_CREATE_PARMS sm_create_parms;
    NBB_MEMSET(&sm_create_parms, 0, sizeof(ASE_CREATE_PARMS));
    sm_create_parms.length = sizeof(ASE_CREATE_PARMS);
    sm_create_parms.is_primary = ATG_YES;
    sm_create_parms.location_capable = ATG_NO;
    sm_create_parms.ft_capable = ATG_NO;
    sm_create_parms.location_index = 1;
    sm_create_parms.location_group_index = 1;
    sm_create_parms.hardware_location = 1;

    // Enable CTM (CSS)
    sm_create_parms.css_enabled = ATG_YES;
    sm_create_parms.css_location_index = 1;
    sm_create_parms.css_location_group_index = 1;
    sm_create_parms.css_hardware_location = 1;
    sm_create_parms.css_entity_index = 1;
    sm_create_parms.css_management_source = ATG_YES;
    sm_create_parms.initiate_start_of_day_replay = ATG_NO;

    /***************************************************************************/
    /* CUSTOMIZE ME.  Define a port-specific section in the SM create parms    */
    /* and replace this code.                                                  */
    /*                                                                         */
    /* This section of System Manager create parms is customer specific.  The  */
    /* asmb_cust_info should be modified as part of the customer port, and any */
    /* relevant values supplied here.  This value is for Metaswitch internal   */
    /* testing purposes.                                                       */
    /***************************************************************************/
    sm_create_parms.asmb_cust_info.def_appl_address = 1;

    /***************************************************************************/
    /* Create System Manager synchronously and store the returned N-BASE       */
    /* process id.  Note that as part of creating System Manager the following */
    /* components are also created.                                            */
    /*                                                                         */
    /* - MIB Manager (this is core product code)                               */
    /* - MIB (SMS) stub (this should be written as part of the customer port). */
    /***************************************************************************/
    NBB_PROC_ID smsi_sm_pid = NBB_NULL_PROC_ID;
    smsi_sm_pid = NBB_CREATE_FTI(PCT_ASM, &sm_create_parms);
    nbase_init = TRUE;

    if (smsi_sm_pid == NBB_NULL_PROC_ID)
    {
        NBB_TRC_FLOW((NBB_FORMAT "Failed to create System Manager."));
        NBB_PRINTF("Failed to create System Manager.\n");
        goto EXIT_LABEL;
    }

    /***************************************************************************/
    /* The MIB Stub will have been created by now as it is automatically       */
    /* created by SM.  Call into the MIB stub to build and queue a series of   */
    /* MIB requests to activate:                                               */
    /*                                                                         */
    /* - Various stubs.                                                        */
    /* - DC-OSPF, DC-ISIS and DC-RTM.                                          */
    /* - DC-LMGR, DC-RSVP and DC_TEMIB and request an LSP for RSVP.            */
    /* - DC-LMGR, DC-CR-LDP Path Manager and DC-CR-LDP Session Controller for  */
    /*   LDP.                                                                  */
    /* - DC-LLDP.                                                              */
    /* - DC-STP.                                                               */
    /*                                                                         */
    /* Note that sms_generate_mib_requests() acts in the context of the        */
    /* MIB (SMS) Stub so release shared context here so that the function can  */
    /* obtain it in the different context.                                     */
    /***************************************************************************/

    NBB_TRC_EXIT();
    NBS_EXIT_SHARED_CONTEXT();

    /***************************************************************************/
    /* Spin N-Base                                                             */
    /***************************************************************************/
    NBS_SPIN_START();

    NBS_ENTER_SHARED_CONTEXT(NBS_ROOT_PROCESS);
    pds_ms::hal_init();
    NBS_EXIT_SHARED_CONTEXT();

    /***************************************************************************/
    /* Register for MIB trap messages                                          */
    /***************************************************************************/
    if (FALSE == pds_ms_register_mib_notify_sink())
    {
        // Failed to register for MIB trap messages
        goto EXIT_LABEL;
    }

    /*************************************************************************/
    /* Create Metaswitch Stubs and Processes                                 */
    /*************************************************************************/
    pds_ms_stubs_create();
    
    /***************************************************************************/
    /* Spin N-Base Again, its stopped in _cs_create_cpi_stub                   */
    /***************************************************************************/
    NBS_SPIN_START();

EXIT_LABEL:
    if (nbase_init)
    {
        /*************************************************************************/
        /* FLOW TRACING NOT REQUIRED    Reason: Performance.                     */
        /*************************************************************************/

        /*************************************************************************/
        /* Enter N-BASE root context.                                            */
        /*************************************************************************/
        NBS_ENTER_SHARED_CONTEXT(NBS_ROOT_PROCESS);

        NBB_TRC_ENTRY("smsi_main");

        NBB_TRC_FLOW((NBB_FORMAT "Terminate N-BASE"));
        /*************************************************************************/
        /* We guarantee the N-BASE has been initialized at this point.           */
        /*************************************************************************/

        /*************************************************************************/
        /* Terminate the systems that started successfully.                      */
        /*************************************************************************/
        //smsi_terminate(NBB_CXT);

        NBB_TRC_EXIT();

        /*************************************************************************/
        /* Exit N-BASE root context.                                             */
        /*************************************************************************/
        NBS_EXIT_SHARED_CONTEXT();

        /*************************************************************************/
        /* Terminate the N-BASE.                                                 */
        /*************************************************************************/
        NBS_TERMINATE();
    }
    return;
}


bool pds_ms_mgmt_init()
{
    mgmt_state_init();
    // Enter thread-safe context to cache nbase thread ptr
    {
        auto ctx = mgmt_state_t::thread_context();
        ctx.state()->set_nbase_thread(sdk::lib::thread::current_thread());
    }
    // Start nbase - blocking call
    nbase_init();
    return true;
}

namespace pds_ms {

static    CPTestSvcImpl        cp_test_svc;
static    BGPSvcImpl            bgp_svc;
static    EvpnSvcImpl           evpn_svc;
static    CPRouteSvcImpl        cp_route_svc;

void
mgmt_svc_init (grpc::ServerBuilder* server_builder)
{
    server_builder->RegisterService(&bgp_svc);
    server_builder->RegisterService(&evpn_svc);
    server_builder->RegisterService(&cp_route_svc);

//    if (PDS_MOCK_MODE()) {
        PDS_TRACE_INFO("Registering CP test proto");
        server_builder->RegisterService(&cp_test_svc);
//    }

    PDS_TRACE_INFO("Mgmt SVC init successful");
}

} // End namespace

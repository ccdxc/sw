// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/vrf_pd.hpp"
#include "nic/hal/pd/iris/nwsec_pd.hpp"
#include "nic/hal/pd/iris/l2seg_pd.hpp"
#include "nic/hal/pd/iris/multicast_pd.hpp"
#include "nic/hal/pd/iris/lif_pd.hpp"
#include "nic/hal/pd/iris/endpoint_pd.hpp"
#include "nic/hal/pd/iris/uplinkif_pd.hpp"
#include "nic/hal/pd/iris/uplinkpc_pd.hpp"
#include "nic/hal/pd/iris/enicif_pd.hpp"
#include "nic/hal/pd/iris/cpuif_pd.hpp"
#include "nic/hal/pd/iris/app_redir_if_pd.hpp"
#include "nic/hal/pd/iris/if_pd.hpp"
#include "nic/hal/pd/iris/tunnelif_pd.hpp"
#include "nic/hal/pd/iris/session_pd.hpp"
#include "nic/hal/pd/iris/qos_pd.hpp"
#include "nic/hal/pd/iris/copp_pd.hpp"
#include "nic/hal/pd/iris/acl_pd.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/p4pd_api.hpp"
#include "nic/gen/iris/include/p4pd.h"
#include "nic/include/hal_pd.hpp"
#include "nic/hal/periodic/periodic.hpp"
#include "nic/include/asic_pd.hpp"
#include "nic/hal/pd/iris/tlscb_pd.hpp"
#include "nic/hal/pd/iris/tcpcb_pd.hpp"
#include "nic/hal/pd/libs/wring/wring_pd.hpp"
#include "nic/hal/src/proxy.hpp"
#include "nic/hal/pd/iris/crypto_keys_pd.hpp"
#include "nic/hal/pd/iris/ipseccb_pd.hpp"
#include "nic/hal/pd/iris/l4lb_pd.hpp"
#include "nic/hal/pd/iris/rw_pd.hpp"
#include "nic/hal/pd/iris/tnnl_rw_pd.hpp"
#include "nic/hal/pd/iris/cpucb_pd.hpp"
#include "nic/hal/pd/cpupkt_api.hpp"
#include "nic/hal/pd/iris/rawrcb_pd.hpp"
#include "nic/hal/pd/capri/capri_barco_res.hpp"
#include "nic/hal/pd/iris/rawccb_pd.hpp"
#include "nic/hal/pd/iris/proxyrcb_pd.hpp"
#include "nic/hal/pd/iris/proxyccb_pd.hpp"
#include "nic/hal/pd/iris/dos_pd.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/capri/capri_tbl_rw.hpp"
#include "nic/hal/pd/asicpd/asic_pd_common.hpp"
#include "nic/hal/pd/asicpd/asic_pd_scheduler.hpp"
#include "nic/p4/iris/include/defines.h"

namespace hal {
extern thread   *g_hal_threads[HAL_THREAD_ID_MAX];
namespace pd {

// P4PD_Lib_Trace Vs Table_Lib_Trace
// ---------------------------------
//  Table_Lib_Trace = !P4PD_Lib_Trace
//  Effectively don't want dual tracing or no tracing.
#define ENTRY_TRACE_EN HAL_LOG_TBL_UPDATES ? false : true

class hal_state_pd *g_hal_state_pd;

//------------------------------------------------------------------------------
// init() function to instantiate all the slabs
//------------------------------------------------------------------------------
bool
hal_state_pd::init(void)
{
    // initialize vrf related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_VRF_PD)] = 
        slab::factory("Vrf PD", HAL_SLAB_VRF_PD,
                      sizeof(hal::pd::pd_vrf_t), 8,
                      false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_VRF_PD)] != NULL), 
                      false);

    vrf_hwid_idxr_ = sdk::lib::indexer::factory(HAL_MAX_HW_VRFS,
                                                  true, /* thread safe */ 
                                                  true); /*skip zero */
    HAL_ASSERT_RETURN((vrf_hwid_idxr_ != NULL), false);
    // vrf_hwid_idxr_->alloc_withid(0);

    // initialize security related data structures
    nwsec_profile_hwid_idxr_ =
        sdk::lib::indexer::factory(HAL_MAX_HW_NWSEC_PROFILES);
    HAL_ASSERT_RETURN((nwsec_profile_hwid_idxr_ != NULL), false);

    // initialize l2seg related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_L2SEG_PD)] = 
        slab::factory("L2Segment PD", HAL_SLAB_L2SEG_PD,
                      sizeof(hal::pd::pd_l2seg_t), 8,
                      false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_L2SEG_PD)] != NULL), 
                      false);

    flow_lkupid_ht_ = ht::factory(HAL_MAX_HW_L2SEGMENTS + HAL_MAX_HW_VRFS,
                                 hal::pd::flow_lkupid_get_hw_key_func,
                                 hal::pd::flow_lkupid_compute_hw_hash_func,
                                 hal::pd::flow_lkupid_compare_hw_key_func);
    HAL_ASSERT_RETURN((flow_lkupid_ht_ != NULL), false);

    l2seg_cpu_idxr_ = sdk::lib::indexer::factory(HAL_MAX_HW_L2SEGMENTS, 
                                                 true, /* thread safe */
                                                 true);/* skip zero */
    HAL_ASSERT_RETURN((l2seg_cpu_idxr_ != NULL), false);

    // initialize mc entry related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_MC_ENTRY_PD)] = 
        slab::factory("mc entry PD", HAL_SLAB_MC_ENTRY_PD,
                      sizeof(hal::pd::pd_mc_entry_t), 8,
                      false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_MC_ENTRY_PD)] != NULL), 
                      false);

    // initialize lport indexer
    lport_idxr_ = sdk::lib::indexer::factory(HAL_MAX_LPORTS, 
                                             true,     /* thread safe */
                                             true);    /* skip zero */
    HAL_ASSERT_RETURN((lport_idxr_ != NULL), false);

    // initialize LIF PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_LIF_PD)] = 
        slab::factory("LIF_PD", HAL_SLAB_LIF_PD,
                      sizeof(hal::pd::pd_lif_t), 8,
                      false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_LIF_PD)] != NULL), 
                      false);

    lif_hwid_idxr_ = sdk::lib::indexer::factory(HAL_MAX_HW_LIFS);
    HAL_ASSERT_RETURN((lif_hwid_idxr_ != NULL), false);

    // initialize Uplink If PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_UPLINKIF_PD)] = 
        slab::factory("UPLINKIF_PD", HAL_SLAB_UPLINKIF_PD,
                      sizeof(hal::pd::pd_uplinkif_t), 8,
                      false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_UPLINKIF_PD)] != NULL), 
                      false);

    // initialize Uplink PC PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_UPLINKPC_PD)] = 
        slab::factory("UPLINKPC_PD", HAL_SLAB_UPLINKPC_PD,
                      sizeof(hal::pd::pd_uplinkpc_t), 8,
                      false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_UPLINKPC_PD)] != NULL), 
                      false);

    uplinkifpc_idxr_ = sdk::lib::indexer::factory(HAL_MAX_UPLINK_IF_PCS);
    HAL_ASSERT_RETURN((uplinkifpc_idxr_ != NULL), false);

    // initialize ENIC If PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_ENICIF_PD)] = 
        slab::factory("ENICIF_PD", HAL_SLAB_ENICIF_PD,
                      sizeof(hal::pd::pd_enicif_t), 8,
                      false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_ENICIF_PD)] != NULL), 
                      false);

    // initialize ENIC If l2seg PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_IF_L2SEG_PD)] = 
        slab::factory("IF_L2SEG_PD", HAL_SLAB_IF_L2SEG_PD,
                      sizeof(hal::pd::pd_if_l2seg_entry_t), 8,
                      false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_IF_L2SEG_PD)] != NULL), 
                      false);

    // initialize CPU If PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_CPUIF_PD)] = 
        slab::factory("CPUIF_PD", HAL_SLAB_CPUIF_PD,
                      sizeof(hal::pd::pd_cpuif_t), 8,
                      false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_CPUIF_PD)] != NULL), 
                      false);

    // initialize App Redirect If PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_APP_REDIR_IF_PD)] = 
        slab::factory("APP_REDIR_IF_PD", 
                      HAL_SLAB_APP_REDIR_IF_PD,
                      sizeof(hal::pd::pd_app_redir_if_t), 
                      2,
                      false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_APP_REDIR_IF_PD)] != NULL), 
                      false);

    // initialize TUNNEL If PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_TUNNELIF_PD)] = 
        slab::factory("TUNNELIF_PD", 
                      HAL_SLAB_TUNNELIF_PD,
                      sizeof(hal::pd::pd_tunnelif_t), 
                      8,
                      false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_TUNNELIF_PD)] != NULL), 
                      false);

    // initialize EP PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_EP_PD)] = 
        slab::factory("EP_PD", HAL_SLAB_EP_PD,
                      sizeof(hal::pd::pd_ep_t), 8,
                      false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_EP_PD)] != NULL), 
                      false);

    // initialize EP PD l3 entry related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_EP_IP_ENTRY_PD)] = 
        slab::factory("EP_PD_IP_ENTRY", 
                      HAL_SLAB_EP_IP_ENTRY_PD,
                      sizeof(hal::pd::pd_ep_ip_entry_t), 8,
                      false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_EP_IP_ENTRY_PD)] != NULL), 
                      false);

    // initialize dos-policy PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_DOS_POLICY_PD)] = 
        slab::factory("NWSEC_PD", HAL_SLAB_DOS_POLICY_PD,
                      sizeof(hal::pd::pd_dos_policy_t), 8,
                      false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_DOS_POLICY_PD)] != NULL), 
                      false);

    // initialize nwsec PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_SECURITY_PROFILE_PD)] = 
        slab::factory("NWSEC_PD", HAL_SLAB_SECURITY_PROFILE_PD,
                      sizeof(hal::pd::pd_nwsec_profile_t), 8,
                      false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_SECURITY_PROFILE_PD)] != NULL), 
                      false);

    // initialize session related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_SESSION_PD)] = 
        slab::factory("Session PD", HAL_SLAB_SESSION_PD,
                      sizeof(hal::pd::pd_session_t), 128,
                      true, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_SESSION_PD)] != NULL), 
                      false);

    // initialize Qos class PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_QOS_CLASS_PD)] = 
        slab::factory("QOS_CLASS_PD", HAL_SLAB_QOS_CLASS_PD,
                      sizeof(hal::pd::pd_qos_class_t), 8,
                      false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_QOS_CLASS_PD)] != NULL), 
                      false);

    qos_txdma_iq_idxr_ = sdk::lib::indexer::factory(HAL_MAX_TXDMA_IQS);
    HAL_ASSERT_RETURN(qos_txdma_iq_idxr_ != NULL, false);

    qos_uplink_iq_idxr_ = sdk::lib::indexer::factory(HAL_MAX_TXDMA_IQS);
    HAL_ASSERT_RETURN(qos_uplink_iq_idxr_ != NULL, false);

    qos_common_oq_idxr_ = sdk::lib::indexer::factory(HAL_MAX_COMMON_OQS);
    HAL_ASSERT_RETURN((qos_common_oq_idxr_ != NULL), false);

    qos_rxdma_oq_idxr_ = sdk::lib::indexer::factory(HAL_MAX_RXDMA_ONLY_OQS);
    HAL_ASSERT_RETURN((qos_rxdma_oq_idxr_ != NULL), false);

    // initialize Copp PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_COPP_PD)] = 
        slab::factory("COPP_PD", HAL_SLAB_COPP_PD,
                      sizeof(hal::pd::pd_copp_t), 8,
                      false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_COPP_PD)] != NULL), 
                      false);

    // initialize TLSCB related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_TLSCB_PD)] = 
        slab::factory("TLSCB PD", HAL_SLAB_TLSCB_PD,
                      sizeof(hal::pd::pd_tlscb_t), 128,
                      true, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_TLSCB_PD)] != NULL), 
                      false);

    tlscb_hwid_ht_ = ht::factory(HAL_MAX_HW_TLSCBS,
                                 hal::pd::tlscb_pd_get_hw_key_func,
                                 hal::pd::tlscb_pd_compute_hw_hash_func,
                                 hal::pd::tlscb_pd_compare_hw_key_func);
    HAL_ASSERT_RETURN((tlscb_hwid_ht_ != NULL), false);

    // initialize TCPCB related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_TCPCB_PD)] = 
        slab::factory("TCPCB PD", HAL_SLAB_TCPCB_PD,
                      sizeof(hal::pd::pd_tcpcb_t), 128,
                      true, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_TCPCB_PD)] != NULL), 
                      false);

    tcpcb_hwid_ht_ = ht::factory(HAL_MAX_HW_TCPCBS,
                                 hal::pd::tcpcb_pd_get_hw_key_func,
                                 hal::pd::tcpcb_pd_compute_hw_hash_func,
                                 hal::pd::tcpcb_pd_compare_hw_key_func);
    HAL_ASSERT_RETURN((tcpcb_hwid_ht_ != NULL), false);

    // initialize Acl PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_ACL_PD)] = 
        slab::factory("ACL_PD", HAL_SLAB_ACL_PD,
                      sizeof(hal::pd::pd_acl_t), 8,
                      false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_ACL_PD)] != NULL), 
                      false);

    // initialize WRING related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_WRING_PD)] = 
        slab::factory("WRING PD", HAL_SLAB_WRING_PD,
                      sizeof(hal::pd::pd_wring_t), 128,
                      true, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_WRING_PD)] != NULL), 
                      false);

    wring_hwid_ht_ = ht::factory(HAL_MAX_HW_WRING,
                                 hal::pd::wring_pd_get_hw_key_func,
                                 hal::pd::wring_pd_compute_hw_hash_func,
                                 hal::pd::wring_pd_compare_hw_key_func);
    HAL_ASSERT_RETURN((wring_hwid_ht_ != NULL), false);

    // Indexer based allocator to manage the crypto session keys
    session_keys_idxr_ = sdk::lib::indexer::factory(CRYPTO_KEY_COUNT_MAX);
    HAL_ASSERT_RETURN((session_keys_idxr_ != NULL), false);

    // initialize IPSECCB related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_IPSECCB_PD)] = 
        slab::factory("IPSECCB PD", HAL_SLAB_IPSECCB_PD,
                      sizeof(hal::pd::pd_ipseccb_encrypt_t), 128,
                      true, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_IPSECCB_PD)] != NULL), 
                      false);

    ipseccb_hwid_ht_ = ht::factory(HAL_MAX_HW_IPSECCBS,
                                 hal::pd::ipseccb_pd_get_hw_key_func,
                                 hal::pd::ipseccb_pd_compute_hw_hash_func,
                                 hal::pd::ipseccb_pd_compare_hw_key_func);
    HAL_ASSERT_RETURN((ipseccb_hwid_ht_ != NULL), false);

    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_IPSECCB_DECRYPT_PD)] = 
        slab::factory("IPSECCB PD", 
                      HAL_SLAB_IPSECCB_DECRYPT_PD,
                      sizeof(hal::pd::pd_ipseccb_decrypt_t), 128,
                      true, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_IPSECCB_DECRYPT_PD)] != NULL), 
                      false);

    ipseccb_decrypt_hwid_ht_ = ht::factory(HAL_MAX_HW_IPSECCBS,
                                 hal::pd::ipseccb_pd_decrypt_get_hw_key_func,
                                 hal::pd::ipseccb_pd_decrypt_compute_hw_hash_func,
                                 hal::pd::ipseccb_pd_decrypt_compare_hw_key_func);
    HAL_ASSERT_RETURN((ipseccb_decrypt_hwid_ht_ != NULL), false);

    // initialize L4LB PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_L4LB_PD)] = 
        slab::factory("L4LB_PD", HAL_SLAB_L4LB_PD,
                      sizeof(hal::pd::pd_l4lb_t), 8,
                      false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_L4LB_PD)] != NULL), 
                      false);

    // initialize rw table management structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_RW_PD)] = 
        slab::factory("RW TBL", HAL_SLAB_RW_PD,
                      sizeof(hal::pd::pd_rw_entry_t), 128,
                      true, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_RW_PD)] != NULL), 
                      false);

    rw_table_ht_ = ht::factory(HAL_MAX_RW_TBL_ENTRIES,
                               hal::pd::rw_entry_pd_get_key_func,
                               hal::pd::rw_entry_pd_compute_hash_func,
                               hal::pd::rw_entry_pd_compare_key_func);
    HAL_ASSERT_RETURN((rw_table_ht_ != NULL), false);

    rw_tbl_idxr_ = sdk::lib::indexer::factory(HAL_MAX_RW_TBL_ENTRIES);
    HAL_ASSERT_RETURN((rw_tbl_idxr_ != NULL), false);

    // initialize tunnel rw table management structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_TUNNEL_RW_PD)] = 
        slab::factory("TUNNEL RW TBL", HAL_SLAB_TUNNEL_RW_PD,
                      sizeof(hal::pd::pd_tnnl_rw_entry_t), 128,
                      true, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_TUNNEL_RW_PD)] != NULL), 
                      false);

    tnnl_rw_table_ht_ = ht::factory(HAL_TUNNEL_RW_TABLE_SIZE,
                               hal::pd::tnnl_rw_entry_pd_get_key_func,
                               hal::pd::tnnl_rw_entry_pd_compute_hash_func,
                               hal::pd::tnnl_rw_entry_pd_compare_key_func);
    HAL_ASSERT_RETURN((tnnl_rw_table_ht_ != NULL), false);

    tnnl_rw_tbl_idxr_ = sdk::lib::indexer::factory(HAL_TUNNEL_RW_TABLE_SIZE);
    HAL_ASSERT_RETURN((tnnl_rw_tbl_idxr_ != NULL), false);
    
    // initialize CPUCB related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_CPUCB_PD)] = 
        slab::factory("CPUCB PD", HAL_SLAB_CPUCB_PD,
                      sizeof(hal::pd::pd_cpucb_t), 128,
                      true, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_CPUCB_PD)] != NULL), 
                      false);

    cpucb_hwid_ht_ = ht::factory(HAL_MAX_HW_CPUCBS,
                                 hal::pd::cpucb_pd_get_hw_key_func,
                                 hal::pd::cpucb_pd_compute_hw_hash_func,
                                 hal::pd::cpucb_pd_compare_hw_key_func);
    HAL_ASSERT_RETURN((cpucb_hwid_ht_ != NULL), false);

    // initialize CPUPKT related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_CPUPKT_PD)] = 
        slab::factory("CPUPKT PD", HAL_SLAB_CPUPKT_PD,
                      sizeof(hal::pd::cpupkt_ctxt_t), MAX_CPU_PKT_QUEUES,
                      true, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_CPUPKT_PD)] != NULL), 
                      false);
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_CPUPKT_QINST_INFO_PD)] = 
        slab::factory("CPUPKT QINFO PD",
                      HAL_SLAB_CPUPKT_QINST_INFO_PD,
                      sizeof(hal::pd::cpupkt_queue_info_t),
                      MAX_CPU_PKT_QUEUES,
                      true, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_CPUPKT_QINST_INFO_PD)] != NULL), 
                      false);

    cpupkt_descr_hwid_idxr_ = sdk::lib::indexer::factory(HAL_MAX_CPU_PKT_DESCR_ENTRIES);
    HAL_ASSERT_RETURN((cpupkt_descr_hwid_idxr_ != NULL), false);
    cpupkt_page_hwid_idxr_ = sdk::lib::indexer::factory(HAL_MAX_CPU_PKT_PAGE_ENTRIES);
    HAL_ASSERT_RETURN((cpupkt_page_hwid_idxr_ != NULL), false);

    // initialize RAWRCB related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_RAWRCB_PD)] = 
        slab::factory("RAWRCB PD", HAL_SLAB_RAWRCB_PD,
                      sizeof(hal::pd::pd_rawrcb_t), 128,
                      true, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_RAWRCB_PD)] != NULL), 
                      false);

    rawrcb_hwid_ht_ = ht::factory(HAL_MAX_HW_RAWRCB_HT_SIZE,
                                  hal::pd::rawrcb_pd_get_hw_key_func,
                                  hal::pd::rawrcb_pd_compute_hw_hash_func,
                                  hal::pd::rawrcb_pd_compare_hw_key_func);
    HAL_ASSERT_RETURN((rawrcb_hwid_ht_ != NULL), false);

    // initialize PROXYRCB related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_PROXYRCB_PD)] = 
        slab::factory("PROXYRCB PD", HAL_SLAB_PROXYRCB_PD,
                      sizeof(hal::pd::pd_proxyrcb_t), 128,
                      true, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_PROXYRCB_PD)] != NULL), false);

    proxyrcb_hwid_ht_ = ht::factory(HAL_MAX_HW_PROXYRCB_HT_SIZE,
                                    hal::pd::proxyrcb_pd_get_hw_key_func,
                                    hal::pd::proxyrcb_pd_compute_hw_hash_func,
                                    hal::pd::proxyrcb_pd_compare_hw_key_func);
    HAL_ASSERT_RETURN((proxyrcb_hwid_ht_ != NULL), false);

    // initialize RAWCCB related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_RAWCCB_PD)] = 
        slab::factory("RAWCCB PD", HAL_SLAB_RAWCCB_PD,
                      sizeof(hal::pd::pd_rawccb_t), 128,
                      true, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_RAWCCB_PD)] != NULL), 
                      false);

    rawccb_hwid_ht_ = ht::factory(HAL_MAX_HW_RAWCCB_HT_SIZE,
                                  hal::pd::rawccb_pd_get_hw_key_func,
                                  hal::pd::rawccb_pd_compute_hw_hash_func,
                                  hal::pd::rawccb_pd_compare_hw_key_func);
    HAL_ASSERT_RETURN((rawccb_hwid_ht_ != NULL), false);

    // initialize PROXYCCB related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_PROXYCCB_PD)] = 
        slab::factory("PROXYCCB PD", HAL_SLAB_PROXYCCB_PD,
                      sizeof(hal::pd::pd_proxyccb_t), 128,
                      true, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_PROXYCCB_PD)] != NULL), 
                      false);

    proxyccb_hwid_ht_ = ht::factory(HAL_MAX_HW_PROXYCCB_HT_SIZE,
                                    hal::pd::proxyccb_pd_get_hw_key_func,
                                    hal::pd::proxyccb_pd_compute_hw_hash_func,
                                    hal::pd::proxyccb_pd_compare_hw_key_func);
    HAL_ASSERT_RETURN((proxyccb_hwid_ht_ != NULL), false);

    dm_tables_ = NULL;
    hash_tcam_tables_ = NULL;
    tcam_tables_ = NULL;
    flow_table_ = NULL;
    met_table_ = NULL;
    acl_table_ = NULL;

    p4plus_rxdma_dm_tables_ = NULL;

    p4plus_txdma_dm_tables_ = NULL;

    return true;
}

//------------------------------------------------------------------------------
// (private) constructor method
//------------------------------------------------------------------------------
hal_state_pd::hal_state_pd()
{
    // slab
    memset(slabs_, 0, sizeof(slabs_));
    
    // indexer
    vrf_hwid_idxr_           = NULL;
    nwsec_profile_hwid_idxr_ = NULL;
    l2seg_cpu_idxr_          = NULL;
    lport_idxr_              = NULL;
    lif_hwid_idxr_           = NULL;
    uplinkifpc_idxr_         = NULL;
    qos_txdma_iq_idxr_       = NULL;
    qos_uplink_iq_idxr_      = NULL;
    qos_common_oq_idxr_      = NULL;
    qos_rxdma_oq_idxr_       = NULL;
    rw_tbl_idxr_             = NULL;
    cpupkt_descr_hwid_idxr_  = NULL;
    cpupkt_page_hwid_idxr_   = NULL;

    // ht
    flow_lkupid_ht_          = NULL;
    tlscb_hwid_ht_           = NULL;
    tcpcb_hwid_ht_           = NULL;
    wring_hwid_ht_           = NULL;
    ipseccb_hwid_ht_         = NULL;
    ipseccb_decrypt_hwid_ht_ = NULL;
    rw_table_ht_             = NULL;
    cpucb_hwid_ht_           = NULL;
    rawrcb_hwid_ht_          = NULL;
    rawccb_hwid_ht_          = NULL;
    proxyrcb_hwid_ht_        = NULL;
    proxyccb_hwid_ht_        = NULL;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
hal_state_pd::~hal_state_pd()
{
    uint32_t    tid;
    for (uint32_t i = HAL_SLAB_PD_MIN; i < HAL_SLAB_PD_MAX; i++) {
        if (slabs_[HAL_PD_SLAB_ID(i)]) {
            slab::destroy(slabs_[HAL_PD_SLAB_ID(i)]);
        }
    }

    vrf_hwid_idxr_ ? indexer::destroy(vrf_hwid_idxr_) : HAL_NOP;
    nwsec_profile_hwid_idxr_ ? indexer::destroy(nwsec_profile_hwid_idxr_) : HAL_NOP;
    l2seg_cpu_idxr_ ? indexer::destroy(l2seg_cpu_idxr_) : HAL_NOP;
    lport_idxr_ ? indexer::destroy(lport_idxr_) : HAL_NOP;
    lif_hwid_idxr_ ? indexer::destroy(lif_hwid_idxr_) : HAL_NOP;
    uplinkifpc_idxr_ ? indexer::destroy(uplinkifpc_idxr_): HAL_NOP;
    qos_uplink_iq_idxr_ ? indexer::destroy(qos_uplink_iq_idxr_) : HAL_NOP;
    qos_txdma_iq_idxr_ ? indexer::destroy(qos_txdma_iq_idxr_) : HAL_NOP;
    qos_common_oq_idxr_ ? indexer::destroy(qos_common_oq_idxr_) : HAL_NOP;
    qos_rxdma_oq_idxr_ ? indexer::destroy(qos_rxdma_oq_idxr_) : HAL_NOP;
    rw_tbl_idxr_ ? indexer::destroy(rw_tbl_idxr_) : HAL_NOP;
    cpupkt_descr_hwid_idxr_ ? indexer::destroy(cpupkt_descr_hwid_idxr_) : HAL_NOP;
    cpupkt_page_hwid_idxr_ ? indexer::destroy(cpupkt_page_hwid_idxr_) : HAL_NOP;

    flow_lkupid_ht_ ? ht::destroy(flow_lkupid_ht_) : HAL_NOP;
    tlscb_hwid_ht_ ? ht::destroy(tlscb_hwid_ht_) : HAL_NOP;
    tcpcb_hwid_ht_ ? ht::destroy(tcpcb_hwid_ht_) : HAL_NOP;
    wring_hwid_ht_ ? ht::destroy(wring_hwid_ht_) : HAL_NOP;
    ipseccb_hwid_ht_ ? ht::destroy(ipseccb_hwid_ht_) : HAL_NOP;
    ipseccb_decrypt_hwid_ht_ ? ht::destroy(ipseccb_decrypt_hwid_ht_) : HAL_NOP;
    cpucb_hwid_ht_ ? ht::destroy(cpucb_hwid_ht_) : HAL_NOP;
    rw_table_ht_ ? ht::destroy(rw_table_ht_) : HAL_NOP;
    rawrcb_hwid_ht_ ? ht::destroy(rawrcb_hwid_ht_) : HAL_NOP;
    rawccb_hwid_ht_ ? ht::destroy(rawccb_hwid_ht_) : HAL_NOP;
    proxyrcb_hwid_ht_ ? ht::destroy(proxyrcb_hwid_ht_) : HAL_NOP;
    proxyccb_hwid_ht_ ? ht::destroy(proxyccb_hwid_ht_) : HAL_NOP;

    if (dm_tables_) {
        for (tid = P4TBL_ID_INDEX_MIN; tid < P4TBL_ID_INDEX_MAX; tid++) {
            if (dm_tables_[tid]) {
                directmap::destroy(dm_tables_[tid]);
            }
        }
        HAL_FREE(HAL_MEM_ALLOC_PD, dm_tables_);
    }

    if (hash_tcam_tables_) {
        for (tid = P4TBL_ID_HASH_OTCAM_MIN;
             tid < P4TBL_ID_HASH_OTCAM_MAX; tid++) {
            if (hash_tcam_tables_[tid]) {
                sdk_hash::destroy(hash_tcam_tables_[tid]);
            }
        }
        HAL_FREE(HAL_MEM_ALLOC_PD, hash_tcam_tables_);
    }

    if (tcam_tables_) {
        for (tid = P4TBL_ID_TCAM_MIN; tid < P4TBL_ID_TCAM_MIN; tid++) {
            if (tcam_tables_[tid]) {
                tcam::destroy(tcam_tables_[tid]);
            }
        }
        HAL_FREE(HAL_MEM_ALLOC_PD, tcam_tables_);
    }

    if (flow_table_) {
        Flow::destroy(flow_table_);
    }

    if (met_table_) {
        Met::destroy(met_table_);
    }

    if (acl_table_) {
        delete acl_table_;
    }

    if (p4plus_rxdma_dm_tables_) {
        for (tid = P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN;
             tid < P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MAX; tid++) {
            if (p4plus_rxdma_dm_tables_[tid - P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN]) {
                directmap::destroy(p4plus_rxdma_dm_tables_[tid - 
                                   P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN]);
            }
        }
        HAL_FREE(HAL_MEM_ALLOC_PD, p4plus_rxdma_dm_tables_);
    }

    if (p4plus_txdma_dm_tables_) {
        for (tid = P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN;
             tid < P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MAX; tid++) {
            if (p4plus_txdma_dm_tables_[tid - P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN]) {
                directmap::destroy(p4plus_txdma_dm_tables_[tid - 
                                   P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN]);
            }
        }
        HAL_FREE(HAL_MEM_ALLOC_PD, p4plus_txdma_dm_tables_);
    }
}

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
hal_state_pd *
hal_state_pd::factory(void)
{
    hal_state_pd *state;

    state = new hal_state_pd();
    HAL_ASSERT_RETURN((state != NULL), NULL);
    if (state->init() == false) {
        delete state;
        return NULL;
    }
    return state;
}

// ----------------------------------------------------------------------------
// Gives slab for a slab id
// ----------------------------------------------------------------------------
#define GET_SLAB(slab_name) \
    if (slab_name && slab_name->slab_id() == slab_id) { \
        return slab_name; \
    }

hal_ret_t
pd_get_slab (pd_get_slab_args_t *args)
{

    args->slab = g_hal_state_pd->get_slab(args->slab_id);
    return HAL_RET_OK;
}

slab *
hal_state_pd::get_slab (hal_slab_t slab_id)
{
    if (slab_id >= HAL_SLAB_PD_MAX || slab_id < HAL_SLAB_PD_MIN) {
        return NULL;
    }
    return slabs_[HAL_PD_SLAB_ID(slab_id)];
}

//------------------------------------------------------------------------------
// convert P4 table type to corresponding string
//------------------------------------------------------------------------------
static const char *
p4pd_table_type2str (p4pd_table_type_en table_type)
{
    switch (table_type) {
    case P4_TBL_TYPE_HASH:
        return "EXACT_HASH";
        break;

    case P4_TBL_TYPE_HASHTCAM:
        return "EXACT_HASH_OTCAM";
        break;

    case P4_TBL_TYPE_TCAM:
        return "TCAM";
        break;

    case P4_TBL_TYPE_INDEX:
        return "EXACT_IDX";
        break;

    case P4_TBL_TYPE_MPU:
        return "MPU";
        break;

    default:
        return "Invalid";
    }
}

#define P4PD_NUM_INGRESS_STAGES        6
#define P4PD_NUM_EGRESS_STAGES         6

//------------------------------------------------------------------------------
// dump all P4 PD table detailed information
//------------------------------------------------------------------------------
static void
p4pd_table_info_dump_ (void)
{
    uint32_t                   tid, stage = 0;
    p4pd_table_properties_t    tinfo;

    // print the header
    printf("=====================================================\nn");
    printf("%-25s\tId\t%-18sIn/Egress\tStage\tStage Tbl Id\tSize\n",
           "Table", "Type");
    printf("======================================================\n");

    // dump ingress tables stage-by-stage first
    while (stage < P4PD_NUM_INGRESS_STAGES) {
        for (tid = P4TBL_ID_TBLMIN; tid < P4TBL_ID_TBLMAX; tid++) {
            p4pd_table_properties_get(tid, &tinfo);
            if ((tinfo.gress != P4_GRESS_INGRESS) || (tinfo.stage != stage)) {
                continue;
            }
            printf("%-25s\t%u\t%-18s%s\t\t%u\t%u\t\t%u\n",
                   tinfo.tablename, tid,
                   p4pd_table_type2str(tinfo.table_type),
                   (tinfo.gress == P4_GRESS_INGRESS) ? "ING" : "EGR",
                   tinfo.stage, tinfo.stage_tableid, tinfo.tabledepth);
        }
        stage++;
    }

    // dump engress tables stage-by-stage now
    stage = 0;
    while (stage < P4PD_NUM_EGRESS_STAGES) {
        for (tid = P4TBL_ID_TBLMIN; tid < P4TBL_ID_TBLMAX; tid++) {
            p4pd_table_properties_get(tid, &tinfo);
            if ((tinfo.gress != P4_GRESS_EGRESS) || (tinfo.stage != stage)) {
                continue;
            }
            printf("%-25s\t%u\t%-18s%s\t\t%u\t%u\t\t%u\n",
                   tinfo.tablename, tid,
                   p4pd_table_type2str(tinfo.table_type),
                   (tinfo.gress == P4_GRESS_INGRESS) ? "ING" : "EGR",
                   tinfo.stage, tinfo.stage_tableid, tinfo.tabledepth);
        }
        stage++;
    }
    printf("======================================================\n");
}

//------------------------------------------------------------------------------
// initializing tables
//------------------------------------------------------------------------------
hal_ret_t
hal_state_pd::init_tables(pd_mem_init_args_t *args)
{
    uint32_t                   tid;
    hal_ret_t                  ret = HAL_RET_OK;
    p4pd_table_properties_t    tinfo, ctinfo;
    p4pd_cfg_t                 p4pd_cfg = {
        .table_map_cfg_file = "iris/capri_p4_table_map.json",
        .p4pd_pgm_name      = "iris",
        .cfg_path           = args->cfg_path,
    };

    memset(&tinfo, 0, sizeof(tinfo));
    memset(&ctinfo, 0, sizeof(ctinfo));

    // parse the NCC generated table info file
    p4pd_init(&p4pd_cfg);

    // start instantiating tables based on the parsed information
    dm_tables_ =
        (directmap **)HAL_CALLOC(HAL_MEM_ALLOC_PD,
                                 sizeof(directmap *) *
                                 (P4TBL_ID_INDEX_MAX - P4TBL_ID_INDEX_MIN + 1));
    HAL_ASSERT(dm_tables_ != NULL);

    // make sure there is one flow table only
    HAL_ASSERT((P4TBL_ID_HASH_MAX - P4TBL_ID_HASH_MIN + 1) == 2);

    hash_tcam_tables_ =
        (sdk_hash **)HAL_CALLOC(HAL_MEM_ALLOC_PD,
                         sizeof(sdk_hash *) *
                         (P4TBL_ID_HASH_OTCAM_MAX - P4TBL_ID_HASH_OTCAM_MIN + 1));
    HAL_ASSERT(hash_tcam_tables_ != NULL);

    tcam_tables_ =
        (tcam **)HAL_CALLOC(HAL_MEM_ALLOC_PD,
                            sizeof(tcam *) *
                            (P4TBL_ID_TCAM_MAX - P4TBL_ID_TCAM_MIN + 1));
    HAL_ASSERT(tcam_tables_ != NULL);

    met_table_ = Met::factory(P4_REPL_TABLE_NAME, P4_REPL_TABLE_ID,
                              P4_REPL_TABLE_DEPTH,
                              CAPRI_REPL_NUM_P4_ENTRIES_PER_NODE,
                              P4_REPL_ENTRY_WIDTH);
    HAL_ASSERT(met_table_ != NULL);

    // for debugging
    p4pd_table_info_dump_();

    for (tid = P4TBL_ID_TBLMIN; tid < P4TBL_ID_TBLMAX; tid++) {
        p4pd_table_properties_get(tid, &tinfo);
        switch (tinfo.table_type) {
        case P4_TBL_TYPE_HASHTCAM:
            if (tinfo.has_oflow_table) {
                p4pd_table_properties_get(tinfo.oflow_table_id, &ctinfo);
            }
            hash_tcam_tables_[tid - P4TBL_ID_HASH_OTCAM_MIN] =
                sdk_hash::factory(tinfo.tablename, tid,
                              tinfo.oflow_table_id,
                              tinfo.tabledepth,
                              tinfo.has_oflow_table ? ctinfo.tabledepth : 0,
                              tinfo.key_struct_size,
                              tinfo.actiondata_struct_size,
                              static_cast<sdk_hash::HashPoly>(tinfo.hash_type),
                              ENTRY_TRACE_EN);
            HAL_ASSERT(hash_tcam_tables_[tid - P4TBL_ID_HASH_OTCAM_MIN] != NULL);
            break;

        case P4_TBL_TYPE_TCAM:
            if (tid == P4TBL_ID_NACL) {
                // ACL tcam table with priority 0 as highest and no duplicate
                // priorities
                acl_table_ = acl_tcam::factory(tinfo.tablename, tid, tinfo.tabledepth,
                                               tinfo.key_struct_size, 
                                               tinfo.actiondata_struct_size, 
                                               false, false);
                HAL_ASSERT(acl_table_ != NULL);
            } else if ((tid == P4TBL_ID_DDOS_SRC_VF) ||
                       (tid == P4TBL_ID_DDOS_SRC_DST) ||
                       (tid == P4TBL_ID_DDOS_SERVICE)) {
                // Allow dup entries must be set to true for ddos tcam tables
                if (!tinfo.is_oflow_table) {
                    tcam_tables_[tid - P4TBL_ID_TCAM_MIN] =
                        tcam::factory(tinfo.tablename, tid, tinfo.tabledepth,
                                      tinfo.key_struct_size, tinfo.actiondata_struct_size, true,
                                      ENTRY_TRACE_EN);
                    HAL_ASSERT(tcam_tables_[tid - P4TBL_ID_TCAM_MIN] != NULL);
                }
            } else {
                if (!tinfo.is_oflow_table) {
                    tcam_tables_[tid - P4TBL_ID_TCAM_MIN] =
                        tcam::factory(tinfo.tablename, tid, tinfo.tabledepth,
                                      tinfo.key_struct_size, tinfo.actiondata_struct_size, false,
                                      ENTRY_TRACE_EN);
                    HAL_ASSERT(tcam_tables_[tid - P4TBL_ID_TCAM_MIN] != NULL);
                }
            }

            break;

        case P4_TBL_TYPE_INDEX:
            if (tid == P4TBL_ID_TWICE_NAT) {
                dm_tables_[tid - P4TBL_ID_INDEX_MIN] =
                    directmap::factory(tinfo.tablename, tid, tinfo.tabledepth,
                                       tinfo.actiondata_struct_size, true, 
                                       ENTRY_TRACE_EN);
            } else {
                dm_tables_[tid - P4TBL_ID_INDEX_MIN] =
                    directmap::factory(tinfo.tablename, tid, tinfo.tabledepth, 
                                       tinfo.actiondata_struct_size, false,
                                       ENTRY_TRACE_EN);
            }
            HAL_ASSERT(dm_tables_[tid - P4TBL_ID_INDEX_MIN] != NULL);
            break;

        case P4_TBL_TYPE_HASH:
            if (tid == P4TBL_ID_FLOW_HASH_OVERFLOW) {
                break;
            }
            HAL_ASSERT(tid == P4TBL_ID_FLOW_HASH);
            if (tinfo.has_oflow_table) {
                p4pd_table_properties_get(tinfo.oflow_table_id, &ctinfo);
            }
            flow_table_ =
                Flow::factory(tinfo.tablename, tid, tinfo.oflow_table_id,
                              tinfo.tabledepth, ctinfo.tabledepth,
                              tinfo.key_struct_size,
                              sizeof(p4pd_flow_hash_data_t), P4_FLOW_NUM_HINTS_PER_ENTRY,
                              static_cast<Flow::HashPoly>(tinfo.hash_type),
                              HAL_MEM_ALLOC_FLOW, ENTRY_TRACE_EN);
            HAL_ASSERT(flow_table_ != NULL);
            break;

        case P4_TBL_TYPE_MPU:
        default:
            break;
        }
    }

    return ret;
}

hal_ret_t
hal_state_pd::p4plus_rxdma_init_tables(pd_mem_init_args_t *args)
{
    uint32_t                   tid;
    hal_ret_t                  ret = HAL_RET_OK;
    p4pd_table_properties_t    tinfo, ctinfo;
    p4pd_error_t               rc;
    p4pd_cfg_t                 p4pd_cfg = {
        .table_map_cfg_file = "iris/capri_p4_rxdma_table_map.json",
        .p4pd_pgm_name      = "iris",
        .cfg_path           = args->cfg_path,
    };

    memset(&tinfo, 0, sizeof(tinfo));
    memset(&ctinfo, 0, sizeof(ctinfo));

    // parse the NCC generated table info file for p4+ tables
    rc = p4pluspd_rxdma_init(&p4pd_cfg);
    HAL_ASSERT(rc == P4PD_SUCCESS);

    // start instantiating tables based on the parsed information
    p4plus_rxdma_dm_tables_ =
        (directmap **)HAL_CALLOC(HAL_MEM_ALLOC_PD,
                                 sizeof(directmap *) *
                                 (P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MAX -
                                  P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN + 1));
    HAL_ASSERT(p4plus_rxdma_dm_tables_ != NULL);

    // TODO:
    // 1. take care of instantiating flow_table_, acl_table_ and met_table_
    // 2. When tables are instantiated proper names are not passed today,
    // waiting for an API from Mahesh that gives table name given table id

    for (tid = P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMIN;
         tid < P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMAX; tid++) {
        rc = p4pluspd_rxdma_table_properties_get(tid, &tinfo);
        HAL_ASSERT(rc == P4PD_SUCCESS);
        
        switch (tinfo.table_type) {
        case P4_TBL_TYPE_INDEX:
            p4plus_rxdma_dm_tables_[tid - P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN] =
                directmap::factory(tinfo.tablename, tid, tinfo.tabledepth, tinfo.actiondata_struct_size,
                                   false, ENTRY_TRACE_EN);
            HAL_ASSERT(p4plus_rxdma_dm_tables_[tid - P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN] != NULL);
            break;

        case P4_TBL_TYPE_MPU:
        default:
            break;
        }
    }

    return ret;
}

hal_ret_t
hal_state_pd::p4plus_txdma_init_tables(pd_mem_init_args_t *args)
{
    uint32_t                   tid;
    hal_ret_t                  ret = HAL_RET_OK;
    p4pd_table_properties_t    tinfo, ctinfo;
    p4pd_error_t               rc;
    p4pd_cfg_t                 p4pd_cfg = {
        .table_map_cfg_file = "iris/capri_p4_txdma_table_map.json",
        .p4pd_pgm_name      = "iris",
        .cfg_path           = args->cfg_path,
    };

    memset(&tinfo, 0, sizeof(tinfo));
    memset(&ctinfo, 0, sizeof(ctinfo));

    // parse the NCC generated table info file for p4+ tables
    rc = p4pluspd_txdma_init(&p4pd_cfg);
    HAL_ASSERT(rc == P4PD_SUCCESS);

    // start instantiating tables based on the parsed information
    p4plus_txdma_dm_tables_ =
        (directmap **)HAL_CALLOC(HAL_MEM_ALLOC_PD,
                                 sizeof(directmap *) *
                                 (P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MAX -
                                  P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN + 1));
    HAL_ASSERT(p4plus_txdma_dm_tables_ != NULL);

    // TODO:
    // 1. take care of instantiating flow_table_, acl_table_ and met_table_
    // 2. When tables are instantiated proper names are not passed today,
    // waiting for an API from Mahesh that gives table name given table id

    for (tid = P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMIN;
         tid < P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMAX; tid++) {
        rc = p4pluspd_txdma_table_properties_get(tid, &tinfo);
        HAL_ASSERT(rc == P4PD_SUCCESS);
        
        switch (tinfo.table_type) {
        case P4_TBL_TYPE_INDEX:
            p4plus_txdma_dm_tables_[tid - P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN] =
                directmap::factory(tinfo.tablename, tid, tinfo.tabledepth, tinfo.actiondata_struct_size,
                                   false, ENTRY_TRACE_EN);
            HAL_ASSERT(p4plus_txdma_dm_tables_[tid - P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN] != NULL);
            break;

        case P4_TBL_TYPE_MPU:
        default:
            break;
        }
    }

    return ret;
}


//------------------------------------------------------------------------------
// one time memory related initialization for HAL
//------------------------------------------------------------------------------
hal_ret_t
pd_mem_init (pd_mem_init_args_t *args)
{
    hal_ret_t   ret = HAL_RET_OK;

    g_hal_state_pd = hal_state_pd::factory();
    HAL_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

    HAL_TRACE_DEBUG("Initializing p4 asic lib tables ...");
    ret = g_hal_state_pd->init_tables(args);
    if (ret != HAL_RET_OK) {
        delete g_hal_state_pd;
    }

    HAL_TRACE_DEBUG("Initializing p4plus asic lib tables ...");
    ret = g_hal_state_pd->p4plus_rxdma_init_tables(args);
    if (ret != HAL_RET_OK) {
        delete g_hal_state_pd;
    }

    HAL_TRACE_DEBUG("Initializing p4plus asic lib tables ...");
    ret = g_hal_state_pd->p4plus_txdma_init_tables(args);
    if (ret != HAL_RET_OK) {
        delete g_hal_state_pd;
    }

    return ret;
}

//------------------------------------------------------------------------------
// Phase 2 memory related initialization for HAL - after the ASIC is ready
//------------------------------------------------------------------------------
hal_ret_t
pd_mem_init_phase2 (pd_mem_init_phase2_args_t *args)
{
    p4pd_cfg_t    p4pd_cfg = {
        .table_map_cfg_file = "iris/capri_p4_table_map.json",
        .p4pd_pgm_name = "iris"
    };
    asicpd_stats_region_info_t region_arr[] = {{P4TBL_ID_FLOW_STATS, 5},
                                               {P4TBL_ID_RX_POLICER_ACTION, 5},
                                               {P4TBL_ID_COPP_ACTION, 5},
                                               {P4TBL_ID_TX_STATS, 6},
                                               {P4TBL_ID_INGRESS_TX_STATS, 3}};
    int arrlen = sizeof(region_arr)/sizeof(asicpd_stats_region_info_t);
    
    // Capri asic initializations
    // Initialize the p4pd stats region
    HAL_ASSERT(asicpd_p4plus_table_mpu_base_init() == HAL_RET_OK);
    HAL_ASSERT(asicpd_stats_region_init(region_arr, arrlen) == HAL_RET_OK);
    HAL_ASSERT(asicpd_toeplitz_init() == HAL_RET_OK);
    HAL_ASSERT(asicpd_p4plus_table_init() == HAL_RET_OK);
    HAL_ASSERT(asicpd_p4plus_recirc_init() == HAL_RET_OK);
    HAL_ASSERT(asicpd_timer_init() == HAL_RET_OK);
    // Following routines must be called after capri asic init
    HAL_ASSERT(asicpd_table_mpu_base_init(&p4pd_cfg) == HAL_RET_OK);
    HAL_ASSERT(asicpd_program_table_mpu_pc() == HAL_RET_OK);
    HAL_ASSERT(asicpd_deparser_init() == HAL_RET_OK);
    HAL_ASSERT(asicpd_program_hbm_table_base_addr() == HAL_RET_OK);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// program default entries in tables
//------------------------------------------------------------------------------
hal_ret_t
pd_pgm_def_entries (pd_pgm_def_entries_args_t *args)
{
    hal_ret_t   ret = HAL_RET_OK;
    p4pd_def_cfg_t  p4pd_def_cfg;

    HAL_TRACE_DEBUG("Programming table default entries ...");
    p4pd_def_cfg.admin_cos = qos_class_get_admin_cos();
    p4pd_def_cfg.hal_cfg = args->hal_cfg;
    ret = p4pd_table_defaults_init(&p4pd_def_cfg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program default entries, err: {}", ret);
    }

    return ret;
}

//------------------------------------------------------------------------------
// program default entries for P4Plus tables
//------------------------------------------------------------------------------
hal_ret_t
pd_pgm_def_p4plus_entries (pd_pgm_def_p4plus_entries_args_t *args)
{
    hal_ret_t   ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("Programming p4plus default entries ...");
    ret = wring_pd_init_global_rings();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program default rings, err: {}", ret);
        return ret;
    }

    ret = crypto_pd_init();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to intialize Capri Barco err: {}", ret);
        return ret;
    }
    return ret;
}


//------------------------------------------------------------------------------
// free an element back to given PD slab specified by its id
//------------------------------------------------------------------------------
hal_ret_t
free_to_slab (hal_slab_t slab_id, void *elem)
{
    switch (slab_id) {
    case HAL_SLAB_VRF_PD:
        g_hal_state_pd->vrf_slab()->free(elem);
        break;

    case HAL_SLAB_L2SEG_PD:
        g_hal_state_pd->l2seg_slab()->free(elem);
        break;

    case HAL_SLAB_MC_ENTRY_PD:
        g_hal_state_pd->mc_entry_slab()->free(elem);
        break;

    case HAL_SLAB_LIF_PD:
        g_hal_state_pd->lif_pd_slab()->free(elem);
        break;

    case HAL_SLAB_UPLINKIF_PD:
        g_hal_state_pd->uplinkif_pd_slab()->free(elem);
        break;

    case HAL_SLAB_UPLINKPC_PD:
        g_hal_state_pd->uplinkpc_pd_slab()->free(elem);
        break;

    case HAL_SLAB_ENICIF_PD:
        g_hal_state_pd->enicif_pd_slab()->free(elem);
        break;

    case HAL_SLAB_IF_L2SEG_PD:
        g_hal_state_pd->if_l2seg_entry_slab()->free(elem);
        break;

    case HAL_SLAB_CPUIF_PD:
        g_hal_state_pd->cpuif_pd_slab()->free(elem);
        break;

    case HAL_SLAB_APP_REDIR_IF_PD:
        g_hal_state_pd->app_redir_if_pd_slab()->free(elem);
        break;

    case HAL_SLAB_SECURITY_PROFILE_PD:
        g_hal_state_pd->nwsec_pd_slab()->free(elem);
        break;

    case HAL_SLAB_EP_PD:
        g_hal_state_pd->ep_pd_slab()->free(elem);
        break;

    case HAL_SLAB_EP_IP_ENTRY_PD:
        g_hal_state_pd->ep_pd_ip_entry_slab()->free(elem);
        break;

    case HAL_SLAB_SESSION_PD:
        g_hal_state_pd->session_slab()->free(elem);
        break;
 
    case HAL_SLAB_TLSCB_PD:
        g_hal_state_pd->tlscb_slab()->free(elem);
        break;

    case HAL_SLAB_TCPCB_PD:
        g_hal_state_pd->tcpcb_slab()->free(elem);
        break;

    case HAL_SLAB_QOS_CLASS_PD:
        g_hal_state_pd->qos_class_pd_slab()->free(elem);
        break;

    case HAL_SLAB_ACL_PD:
        g_hal_state_pd->acl_pd_slab()->free(elem);
        break;

    case HAL_SLAB_WRING_PD:
        g_hal_state_pd->wring_slab()->free(elem);
        break;

    case HAL_SLAB_IPSECCB_PD:
        g_hal_state_pd->ipseccb_slab()->free(elem);
        break;
 
    case HAL_SLAB_IPSECCB_DECRYPT_PD:
        g_hal_state_pd->ipseccb_decrypt_slab()->free(elem);
        break;
    
    case HAL_SLAB_CPUCB_PD:
        g_hal_state_pd->cpucb_slab()->free(elem);
        break;

    case HAL_SLAB_L4LB_PD:
        g_hal_state_pd->l4lb_pd_slab()->free(elem);
        break;

    case HAL_SLAB_RW_PD:
        g_hal_state_pd->rw_entry_slab()->free(elem);
        break;

    case HAL_SLAB_TUNNEL_RW_PD:
        g_hal_state_pd->tnnl_rw_entry_slab()->free(elem);
        break;

    case HAL_SLAB_CPUPKT_PD:
        g_hal_state_pd->cpupkt_slab()->free(elem);
        break;

    case HAL_SLAB_RAWRCB_PD:
        g_hal_state_pd->rawrcb_slab()->free(elem);
        break;

    case HAL_SLAB_RAWCCB_PD:
        g_hal_state_pd->rawccb_slab()->free(elem);
        break;

    case HAL_SLAB_PROXYRCB_PD:
        g_hal_state_pd->proxyrcb_slab()->free(elem);
        break;

    case HAL_SLAB_PROXYCCB_PD:
        g_hal_state_pd->proxyccb_slab()->free(elem);
        break;

    case HAL_SLAB_CPUPKT_QINST_INFO_PD:
        g_hal_state_pd->cpupkt_qinst_info_slab()->free(elem);
        break;

    case HAL_SLAB_COPP_PD:
        g_hal_state_pd->copp_pd_slab()->free(elem);
        break;

    default:
        HAL_TRACE_ERR("Unknown slab id {}", slab_id);
        HAL_ASSERT(FALSE);
        return HAL_RET_INVALID_ARG;
        break;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// callback invoked by the timerwheel to release an object to its slab
//------------------------------------------------------------------------------
static inline void
pd_slab_delay_delete_cb (void *timer, hal_slab_t slab_id, void *elem)
{
    hal_ret_t    ret;

    if (slab_id < HAL_SLAB_PI_MAX) {
        ret = hal::free_to_slab(slab_id, elem);
    } else if (slab_id < HAL_SLAB_PD_MAX) {
        ret = hal::pd::free_to_slab(slab_id, elem);
    } else {
        HAL_TRACE_ERR("{}: Unexpected slab id {}", __FUNCTION__, slab_id);
        ret = HAL_RET_INVALID_ARG;
    }
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("[{}:{}] Failed to release elem {} to slab id {}",
                      __FUNCTION__, __LINE__, elem, slab_id);
    }

    return;
}

//------------------------------------------------------------------------------
// wrapper function to delay delete slab elements
// NOTE: currently delay delete timeout is 2 seconds, it is expected that any
//       other threads using (a pointer to) this object should be done with this
//       object within this timeout or else this memory can be freed and
//       allocated for other objects and can result in corruptions. Hence, tune
//       this timeout, if needed
//------------------------------------------------------------------------------
hal_ret_t
delay_delete_to_slab (hal_slab_t slab_id, void *elem)
{
    void    *timer_ctxt;

    if (g_delay_delete && hal::periodic::periodic_thread_is_running()) {
        timer_ctxt =
            hal::periodic::timer_schedule(slab_id,
                                          TIME_MSECS_PER_SEC << 1, elem,
                                          (sdk::lib::twheel_cb_t)pd_slab_delay_delete_cb,
                                          false);
        if (!timer_ctxt) {
            return HAL_RET_ERR;
        }
    } else {
        pd_slab_delay_delete_cb(NULL, slab_id, elem);
    }
    return HAL_RET_OK;
}


}    // namespace pd
}    // namespace hal


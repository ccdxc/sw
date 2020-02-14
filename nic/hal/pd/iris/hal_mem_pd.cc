// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/nw/vrf_pd.hpp"
#include "nic/hal/pd/iris/firewall/nwsec_pd.hpp"
#include "nic/hal/pd/iris/nw/l2seg_pd.hpp"
#include "nic/hal/pd/iris/mcast/multicast_pd.hpp"
#include "nic/hal/pd/iris/mcast/repl_decode.hpp"
#include "nic/hal/pd/iris/lif/lif_pd.hpp"
#include "nic/hal/pd/iris/nw/endpoint_pd.hpp"
#include "nic/hal/pd/iris/nw/uplinkif_pd.hpp"
#include "nic/hal/pd/iris/nw/uplinkpc_pd.hpp"
#include "nic/hal/pd/iris/nw/enicif_pd.hpp"
#include "nic/hal/pd/iris/nw/cpuif_pd.hpp"
#include "nic/hal/pd/iris/nw/app_redir_if_pd.hpp"
#include "nic/hal/pd/iris/nw/if_pd.hpp"
#include "nic/hal/pd/iris/nw/tunnelif_pd.hpp"
#include "nic/hal/pd/iris/nw/session_pd.hpp"
#include "nic/hal/pd/iris/aclqos/qos_pd.hpp"
#include "nic/hal/pd/iris/internal/copp_pd.hpp"
#include "nic/hal/pd/iris/aclqos/acl_pd.hpp"
#include "nic/include/pd.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/hal/svc/hal_ext.hpp"
#include "gen/p4gen/p4/include/p4pd.h"
#include "nic/include/hal_pd.hpp"
#include "lib/periodic/periodic.hpp"
#include "nic/include/asic_pd.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/hal/pd/iris/internal/tlscb_pd.hpp"
#include "nic/hal/pd/iris/internal/tcpcb_pd.hpp"
#include "nic/hal/pd/libs/wring/wring_pd.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/pd/iris/internal/crypto_keys_pd.hpp"
#include "nic/hal/pd/iris/nw/rw_pd.hpp"
#include "nic/hal/pd/iris/nw/tnnl_rw_pd.hpp"
#include "nic/hal/pd/iris/nvme/nvme_sesscb_pd.hpp"
#include "nic/hal/pd/iris/nvme/nvme_ns_pd.hpp"
#include "nic/hal/pd/iris/nvme/nvme_sq_pd.hpp"
#include "nic/hal/pd/iris/nvme/nvme_cq_pd.hpp"
#include "nic/hal/pd/iris/nvme/nvme_global_pd.hpp"
#ifdef __x86_64__
#include "nic/hal/pd/iris/l4lb/l4lb_pd.hpp"
#include "nic/hal/pd/iris/internal/ipseccb_pd.hpp"
#include "nic/hal/pd/iris/ipsec/ipsec_pd.hpp"
#endif
#include "nic/hal/pd/iris/internal/cpucb_pd.hpp"
#include "nic/hal/pd/cpupkt_api.hpp"
#include "nic/hal/pd/iris/tcp_proxy/tcp_rings.hpp"
#include "nic/hal/pd/iris/internal/rawrcb_pd.hpp"
#include "nic/sdk/platform/capri/capri_barco_res.hpp"
#include "nic/hal/pd/iris/internal/rawccb_pd.hpp"
#include "nic/hal/pd/iris/internal/proxyrcb_pd.hpp"
#include "nic/hal/pd/iris/internal/proxyccb_pd.hpp"
//#include "nic/hal/pd/iris/dos/dos_pd.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "platform/capri/capri_tbl_rw.hpp"
#include "nic/hal/pd/asicpd/asic_pd_common.hpp"
#include "nic/sdk/asic/pd/scheduler.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/hal/pd/iris/event/hal_event_pd.hpp"
#include "nic/hal/pd/iris/debug/debug_pd.hpp"
#include "nic/hal/pd/iris/debug/snake_pd.hpp"
#include "nic/hal/pd/iris/p4pd_cfg.hpp"
#include "nic/sdk/lib/table/sldirectmap/sldirectmap.hpp"

namespace hal {
namespace pd {

// P4PD_Lib_Trace Vs Table_Lib_Trace
// ---------------------------------
//  Table_Lib_Trace = !P4PD_Lib_Trace
//  Effectively don't want dual tracing or no tracing.
// #define ENTRY_TRACE_EN SDK_LOG_TABLE_WRITE ? false : true
#define ENTRY_TRACE_EN true

class hal_state_pd *g_hal_state_pd;

asicpd_stats_region_info_t g_stats_region_arr[] = {{P4TBL_ID_RX_POLICER_ACTION, 5},
                                               {P4TBL_ID_COPP_ACTION, 5}};
int g_stats_region_arrlen = sizeof(g_stats_region_arr)/sizeof(asicpd_stats_region_info_t);

//------------------------------------------------------------------------------
// init() function to instantiate all the slabs
//------------------------------------------------------------------------------
bool
hal_state_pd::init(void)
{
    // initialize vrf related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_VRF_PD)] =
        slab::factory("vrf_pd", HAL_SLAB_VRF_PD,
                      sizeof(hal::pd::pd_vrf_t), 8,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_VRF_PD)] != NULL),
                      false);

    vrf_hwid_idxr_ = sdk::lib::indexer::factory(HAL_MAX_HW_VRFS,
                                                  true, /* thread safe */
                                                  true); /*skip zero */
    SDK_ASSERT_RETURN((vrf_hwid_idxr_ != NULL), false);
    // vrf_hwid_idxr_->alloc_withid(0);

    // initialize security related data structures
    nwsec_profile_hwid_idxr_ =
        sdk::lib::indexer::factory(HAL_MAX_HW_NWSEC_PROFILES);
    SDK_ASSERT_RETURN((nwsec_profile_hwid_idxr_ != NULL), false);

    // initialize l2seg related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_L2SEG_PD)] =
        slab::factory("l2seg_pd", HAL_SLAB_L2SEG_PD,
                      sizeof(hal::pd::pd_l2seg_t), 8,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_L2SEG_PD)] != NULL),
                      false);

    HAL_HT_CREATE("flow-lookup-id", flow_lkupid_ht_,
                  (HAL_MAX_HW_L2SEGMENTS + HAL_MAX_HW_VRFS) >> 1,
                  hal::pd::flow_lkupid_get_hw_key_func,
                  hal::pd::flow_lkupid_hw_key_size());
    SDK_ASSERT_RETURN((flow_lkupid_ht_ != NULL), false);

    l2seg_cpu_idxr_ = sdk::lib::indexer::factory(HAL_MAX_HW_L2SEGMENTS,
                                                 true, /* thread safe */
                                                 true);/* skip zero */
    SDK_ASSERT_RETURN((l2seg_cpu_idxr_ != NULL), false);

    // initialize mc entry related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_MC_ENTRY_PD)] =
        slab::factory("mc_entry_pd", HAL_SLAB_MC_ENTRY_PD,
                      sizeof(hal::pd::pd_mc_entry_t), 8,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_MC_ENTRY_PD)] != NULL),
                      false);

    // initialize lport indexer
    lport_idxr_ = sdk::lib::indexer::factory(HAL_MAX_LPORTS,
                                             true,     /* thread safe */
                                             true);    /* skip zero */
    SDK_ASSERT_RETURN((lport_idxr_ != NULL), false);

    // initialize LIF PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_LIF_PD)] =
        slab::factory("lif_pd", HAL_SLAB_LIF_PD,
                      sizeof(hal::pd::pd_lif_t), 8,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_LIF_PD)] != NULL),
                      false);

    lif_hwid_idxr_ = sdk::lib::indexer::factory(HAL_MAX_HW_LIFS);
    SDK_ASSERT_RETURN((lif_hwid_idxr_ != NULL), false);

    // initialize Uplink If PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_UPLINKIF_PD)] =
        slab::factory("uplinkif_pd", HAL_SLAB_UPLINKIF_PD,
                      sizeof(hal::pd::pd_uplinkif_t), 8,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_UPLINKIF_PD)] != NULL),
                      false);

    // initialize Uplink PC PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_UPLINKPC_PD)] =
        slab::factory("uplinkpc_pd", HAL_SLAB_UPLINKPC_PD,
                      sizeof(hal::pd::pd_uplinkpc_t), 8,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_UPLINKPC_PD)] != NULL),
                      false);

    uplinkifpc_idxr_ = sdk::lib::indexer::factory(HAL_MAX_UPLINK_IF_PCS);
    SDK_ASSERT_RETURN((uplinkifpc_idxr_ != NULL), false);

    // initialize ENIC If PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_ENICIF_PD)] =
        slab::factory("ENICIF_PD", HAL_SLAB_ENICIF_PD,
                      sizeof(hal::pd::pd_enicif_t), 8,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_ENICIF_PD)] != NULL),
                      false);

    // initialize ENIC If l2seg PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_IF_L2SEG_PD)] =
        slab::factory("if_l2seg_pd", HAL_SLAB_IF_L2SEG_PD,
                      sizeof(hal::pd::pd_if_l2seg_entry_t), 8,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_IF_L2SEG_PD)] != NULL),
                      false);

    // initialize CPU If PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_CPUIF_PD)] =
        slab::factory("cpuif_pd", HAL_SLAB_CPUIF_PD,
                      sizeof(hal::pd::pd_cpuif_t), 8,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_CPUIF_PD)] != NULL),
                      false);

    // initialize App Redirect If PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_APP_REDIR_IF_PD)] =
        slab::factory("app_redir_if_pd",
                      HAL_SLAB_APP_REDIR_IF_PD,
                      sizeof(hal::pd::pd_app_redir_if_t),
                      2,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_APP_REDIR_IF_PD)] != NULL),
                      false);

    // initialize TUNNEL If PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_TUNNELIF_PD)] =
        slab::factory("tunnelif_pd",
                      HAL_SLAB_TUNNELIF_PD,
                      sizeof(hal::pd::pd_tunnelif_t),
                      8,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_TUNNELIF_PD)] != NULL),
                      false);

    // initialize EP PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_EP_PD)] =
        slab::factory("ep_pd", HAL_SLAB_EP_PD,
                      sizeof(hal::pd::pd_ep_t), 8,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_EP_PD)] != NULL),
                      false);

    // initialize EP PD l3 entry related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_EP_IP_ENTRY_PD)] =
        slab::factory("ep_pd_ip_entry",
                      HAL_SLAB_EP_IP_ENTRY_PD,
                      sizeof(hal::pd::pd_ep_ip_entry_t), 8,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_EP_IP_ENTRY_PD)] != NULL),
                      false);

#if 0
    // initialize dos-policy PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_DOS_POLICY_PD)] =
        slab::factory("dos_pol_pd", HAL_SLAB_DOS_POLICY_PD,
                      sizeof(hal::pd::pd_dos_policy_t), 8,
                      false, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_DOS_POLICY_PD)] != NULL),
                      false);
#endif

    // initialize nwsec PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_SECURITY_PROFILE_PD)] =
        slab::factory("nwsec_pd", HAL_SLAB_SECURITY_PROFILE_PD,
                      sizeof(hal::pd::pd_nwsec_profile_t), 8,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_SECURITY_PROFILE_PD)] != NULL),
                      false);

    // initialize session related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_SESSION_PD)] =
        slab::factory("session_pd", HAL_SLAB_SESSION_PD,
                      sizeof(hal::pd::pd_session_t), 128,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_SESSION_PD)] != NULL),
                      false);

    // initialize Qos class PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_QOS_CLASS_PD)] =
        slab::factory("qos_class_pd", HAL_SLAB_QOS_CLASS_PD,
                      sizeof(hal::pd::pd_qos_class_t), 8,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_QOS_CLASS_PD)] != NULL),
                      false);

    qos_txdma_iq_idxr_ = sdk::lib::indexer::factory(HAL_MAX_TXDMA_IQS);
    SDK_ASSERT_RETURN(qos_txdma_iq_idxr_ != NULL, false);

    qos_uplink_iq_idxr_ = sdk::lib::indexer::factory(HAL_MAX_TXDMA_IQS);
    SDK_ASSERT_RETURN(qos_uplink_iq_idxr_ != NULL, false);

    qos_common_oq_idxr_ = sdk::lib::indexer::factory(HAL_MAX_COMMON_OQS);
    SDK_ASSERT_RETURN((qos_common_oq_idxr_ != NULL), false);

    qos_rxdma_oq_idxr_ = sdk::lib::indexer::factory(HAL_MAX_RXDMA_ONLY_OQS);
    SDK_ASSERT_RETURN((qos_rxdma_oq_idxr_ != NULL), false);

    // initialize Copp PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_COPP_PD)] =
        slab::factory("copp_pd", HAL_SLAB_COPP_PD,
                      sizeof(hal::pd::pd_copp_t), 8,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_COPP_PD)] != NULL),
                      false);

    // initialize TLSCB related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_TLSCB_PD)] =
        slab::factory("tlscb_pd", HAL_SLAB_TLSCB_PD,
                      sizeof(hal::pd::pd_tlscb_t), 128,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_TLSCB_PD)] != NULL),
                      false);

    HAL_HT_CREATE("tlscb_hw_id", tlscb_hwid_ht_,
                  HAL_MAX_HW_TLSCBS >> 1,
                  hal::pd::tlscb_pd_get_hw_key_func,
                  hal::pd::tlscb_pd_hw_key_size());
    SDK_ASSERT_RETURN((tlscb_hwid_ht_ != NULL), false);

    // initialize TCPCB related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_TCPCB_PD)] =
        slab::factory("tcpcb_pd", HAL_SLAB_TCPCB_PD,
                      sizeof(hal::pd::pd_tcpcb_t), 128,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_TCPCB_PD)] != NULL),
                      false);

    HAL_HT_CREATE("tcpcb_hw_id", tcpcb_hwid_ht_,
                  HAL_MAX_HW_TCPCBS >> 1,
                  hal::pd::tcpcb_pd_get_hw_key_func,
                  hal::pd::tcpcb_pd_hw_key_size());
    SDK_ASSERT_RETURN((tcpcb_hwid_ht_ != NULL), false);

    // initialize Acl PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_ACL_PD)] =
        slab::factory("acl_pd", HAL_SLAB_ACL_PD,
                      sizeof(hal::pd::pd_acl_t), 8,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_ACL_PD)] != NULL),
                      false);

    // initialize WRING related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_WRING_PD)] =
        slab::factory("wring_pd", HAL_SLAB_WRING_PD,
                      sizeof(hal::pd::pd_wring_t), 128,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_WRING_PD)] != NULL),
                      false);

    HAL_HT_CREATE("wring_hw_id", wring_hwid_ht_,
                  HAL_MAX_HW_WRING >> 1,
                  hal::pd::wring_pd_get_hw_key_func,
                  hal::pd::wring_pd_hw_key_size());
    SDK_ASSERT_RETURN((wring_hwid_ht_ != NULL), false);

#ifdef __x86_64__
    // initialize IPSECCB related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_IPSECCB_PD)] =
        slab::factory("ipseccb_pd", HAL_SLAB_IPSECCB_PD,
                      sizeof(hal::pd::pd_ipseccb_encrypt_t), 128,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_IPSECCB_PD)] != NULL),
                      false);

    HAL_HT_CREATE("ipseccb_hw_id", ipseccb_hwid_ht_,
                  HAL_MAX_HW_IPSECCBS >> 1,
                  hal::pd::ipseccb_pd_get_hw_key_func,
                  hal::pd::ipseccb_pd_hw_key_size());
    SDK_ASSERT_RETURN((ipseccb_hwid_ht_ != NULL), false);

    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_IPSECCB_DECRYPT_PD)] =
        slab::factory("ipseccb_pd",
                      HAL_SLAB_IPSECCB_DECRYPT_PD,
                      sizeof(hal::pd::pd_ipseccb_decrypt_t), 128,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_IPSECCB_DECRYPT_PD)] != NULL),
                      false);

    HAL_HT_CREATE("ipsecde_hw_id", ipseccb_decrypt_hwid_ht_,
                  HAL_MAX_HW_IPSECCBS >> 1,
                  hal::pd::ipseccb_pd_decrypt_get_hw_key_func,
                  hal::pd::ipseccb_pd_decrypt_hw_key_size());
    SDK_ASSERT_RETURN((ipseccb_decrypt_hwid_ht_ != NULL), false);

    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_IPSEC_SA_PD)] =
        slab::factory("ipsecsa_pd", HAL_SLAB_IPSEC_SA_PD,
                      sizeof(hal::pd::pd_ipsec_t), 128,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_IPSEC_SA_PD)] != NULL),
                      false);

    HAL_HT_CREATE("ipsecsa_hw_id", ipsec_sa_hwid_ht_,
                  HAL_MAX_HW_IPSEC_SA >> 1,
                  hal::pd::ipsec_pd_get_hw_key_func,
                  hal::pd::ipsec_pd_hw_key_size());
    SDK_ASSERT_RETURN((ipsec_sa_hwid_ht_ != NULL), false);
#endif

    // initialize rw table management structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_RW_PD)] =
        slab::factory("rw_pd", HAL_SLAB_RW_PD,
                      sizeof(hal::pd::pd_rw_entry_t), 128,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_RW_PD)] != NULL),
                      false);

    HAL_HT_CREATE("rw-table", rw_table_ht_,
                  HAL_MAX_RW_TBL_ENTRIES >> 1,
                  hal::pd::rw_entry_pd_get_key_func,
                  hal::pd::rw_entry_pd_key_size());
    SDK_ASSERT_RETURN((rw_table_ht_ != NULL), false);

    rw_tbl_idxr_ = sdk::lib::indexer::factory(HAL_MAX_RW_TBL_ENTRIES);
    SDK_ASSERT_RETURN((rw_tbl_idxr_ != NULL), false);

    // initialize tunnel rw table management structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_TUNNEL_RW_PD)] =
        slab::factory("tunnel_rw_tbl", HAL_SLAB_TUNNEL_RW_PD,
                      sizeof(hal::pd::pd_tnnl_rw_entry_t), 128,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_TUNNEL_RW_PD)] != NULL),
                      false);

    HAL_HT_CREATE("tunnel-rw-table", tnnl_rw_table_ht_,
                  HAL_TUNNEL_RW_TABLE_SIZE >> 1,
                  hal::pd::tnnl_rw_entry_pd_get_key_func,
                  hal::pd::tnnl_rw_entry_pd_key_size());
    SDK_ASSERT_RETURN((tnnl_rw_table_ht_ != NULL), false);

    tnnl_rw_tbl_idxr_ = sdk::lib::indexer::factory(HAL_TUNNEL_RW_TABLE_SIZE);
    SDK_ASSERT_RETURN((tnnl_rw_tbl_idxr_ != NULL), false);

    // initialize NVME_SESSCB related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_NVME_SESSCB_PD)] =
        slab::factory("nvme_sesscb_pd", HAL_SLAB_NVME_SESSCB_PD,
                      sizeof(hal::pd::pd_nvme_sesscb_t), 16,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_NVME_SESSCB_PD)] != NULL),
                      false);

    HAL_HT_CREATE("nvme_sesscb_hw_id", nvme_sesscb_hwid_ht_,
                  HAL_MAX_HW_NVME_SESSCBS >> 1,
                  hal::pd::nvme_sesscb_pd_get_hw_key_func,
                  hal::pd::nvme_sesscb_pd_hw_key_size());
    SDK_ASSERT_RETURN((nvme_sesscb_hwid_ht_ != NULL), false);

    // initialize NVME_GLOBAL related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_NVME_GLOBAL_PD)] =
        slab::factory("nvme_global_pd", HAL_SLAB_NVME_GLOBAL_PD,
                      sizeof(hal::pd::pd_nvme_global_t), 8,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_NVME_GLOBAL_PD)] != NULL),
                      false);

    // initialize NVME_NS related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_NVME_NS_PD)] =
        slab::factory("nvme_ns_pd", HAL_SLAB_NVME_NS_PD,
                      sizeof(hal::pd::pd_nvme_ns_t), 16,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_NVME_NS_PD)] != NULL),
                      false);

    HAL_HT_CREATE("nvme_ns_hw_id", nvme_ns_hwid_ht_,
                  HAL_MAX_HW_NVME_NS >> 1,
                  hal::pd::nvme_ns_pd_get_hw_key_func,
                  hal::pd::nvme_ns_pd_hw_key_size());
    SDK_ASSERT_RETURN((nvme_ns_hwid_ht_ != NULL), false);


    // initialize NVME_SQ related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_NVME_SQ_PD)] =
        slab::factory("nvme_sq_pd", HAL_SLAB_NVME_SQ_PD,
                      sizeof(hal::pd::pd_nvme_sq_t), 16,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_NVME_SQ_PD)] != NULL),
                      false);

    HAL_HT_CREATE("nvme_sq_hw_id", nvme_sq_hwid_ht_,
                  HAL_MAX_HW_NVME_SQ >> 1,
                  hal::pd::nvme_sq_pd_get_hw_key_func,
                  hal::pd::nvme_sq_pd_hw_key_size());
    SDK_ASSERT_RETURN((nvme_sq_hwid_ht_ != NULL), false);

    // initialize NVME_CQ related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_NVME_CQ_PD)] =
        slab::factory("nvme_cq_pd", HAL_SLAB_NVME_CQ_PD,
                      sizeof(hal::pd::pd_nvme_cq_t), 16,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_NVME_CQ_PD)] != NULL),
                      false);

    HAL_HT_CREATE("nvme_cq_hw_id", nvme_cq_hwid_ht_,
                  HAL_MAX_HW_NVME_CQ >> 1,
                  hal::pd::nvme_cq_pd_get_hw_key_func,
                  hal::pd::nvme_cq_pd_hw_key_size());
    SDK_ASSERT_RETURN((nvme_cq_hwid_ht_ != NULL), false);

#ifdef __x86_64__
    // initialize L4LB PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_L4LB_PD)] =
        slab::factory("l4lb_pd", HAL_SLAB_L4LB_PD,
                      sizeof(hal::pd::pd_l4lb_t), 8,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_L4LB_PD)] != NULL),
                      false);
#endif

    // initialize CPUCB related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_CPUCB_PD)] =
        slab::factory("cpucb_pd", HAL_SLAB_CPUCB_PD,
                      sizeof(hal::pd::pd_cpucb_t), 128,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_CPUCB_PD)] != NULL),
                      false);

    HAL_HT_CREATE("cpucb_hw_id", cpucb_hwid_ht_,
                  HAL_MAX_HW_CPUCBS >> 1,
                  hal::pd::cpucb_pd_get_hw_key_func,
                  hal::pd::cpucb_pd_hw_key_size());
    SDK_ASSERT_RETURN((cpucb_hwid_ht_ != NULL), false);

    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_TCP_RINGS_PD)] =
        slab::factory("tcp_rings_pd", HAL_SLAB_TCP_RINGS_PD,
                      sizeof(hal::pd::tcp_rings_ctxt_t), MAX_TCP_RINGS_CTXT,
                      true, true, true);

    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_TCP_RINGS_PD)] != NULL),
                      false);

    // initialize CPUPKT related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_CPUPKT_PD)] =
        slab::factory("cpupkt_pd", HAL_SLAB_CPUPKT_PD,
                      sizeof(hal::pd::cpupkt_ctxt_t), MAX_CPU_PKT_QUEUES,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_CPUPKT_PD)] != NULL),
                      false);
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_CPUPKT_QINST_INFO_PD)] =
        slab::factory("cpupkt_qinfo_pd",
                      HAL_SLAB_CPUPKT_QINST_INFO_PD,
                      sizeof(hal::pd::cpupkt_queue_info_t),
                      MAX_CPU_PKT_QUEUES,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_CPUPKT_QINST_INFO_PD)] != NULL),
                      false);

    // initialize RAWRCB related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_RAWRCB_PD)] =
        slab::factory("rawrcb_pd", HAL_SLAB_RAWRCB_PD,
                      sizeof(hal::pd::pd_rawrcb_t), 128,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_RAWRCB_PD)] != NULL),
                      false);

    HAL_HT_CREATE("rawrcb_hw_id", rawrcb_hwid_ht_,
                  HAL_MAX_HW_RAWRCB_HT_SIZE >> 1,
                  hal::pd::rawrcb_pd_get_hw_key_func,
                  hal::pd::rawrcb_pd_hw_key_size());
    SDK_ASSERT_RETURN((rawrcb_hwid_ht_ != NULL), false);

    // initialize PROXYRCB related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_PROXYRCB_PD)] =
        slab::factory("proxyrcb_pd", HAL_SLAB_PROXYRCB_PD,
                      sizeof(hal::pd::pd_proxyrcb_t), 128,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_PROXYRCB_PD)] != NULL), false);

    HAL_HT_CREATE("proxyrcb_hw_id", proxyrcb_hwid_ht_,
                  HAL_MAX_HW_PROXYRCB_HT_SIZE >> 1,
                  hal::pd::proxyrcb_pd_get_hw_key_func,
                  hal::pd::proxyrcb_pd_hw_key_size());
    SDK_ASSERT_RETURN((proxyrcb_hwid_ht_ != NULL), false);

    // initialize RAWCCB related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_RAWCCB_PD)] =
        slab::factory("rawccb_pd", HAL_SLAB_RAWCCB_PD,
                      sizeof(hal::pd::pd_rawccb_t), 128,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_RAWCCB_PD)] != NULL),
                      false);

    HAL_HT_CREATE("rawccb_hw_id", rawccb_hwid_ht_,
                  HAL_MAX_HW_RAWCCB_HT_SIZE >> 1,
                  hal::pd::rawccb_pd_get_hw_key_func,
                  hal::pd::rawccb_pd_hw_key_size());
    SDK_ASSERT_RETURN((rawccb_hwid_ht_ != NULL), false);

    // initialize PROXYCCB related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_PROXYCCB_PD)] =
        slab::factory("proxyccb_pd", HAL_SLAB_PROXYCCB_PD,
                      sizeof(hal::pd::pd_proxyccb_t), 128,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_PROXYCCB_PD)] != NULL),
                      false);

    HAL_HT_CREATE("proxyccb-hw-id", proxyccb_hwid_ht_,
                  HAL_MAX_HW_PROXYCCB_HT_SIZE >> 1,
                  hal::pd::proxyccb_pd_get_hw_key_func,
                  hal::pd::proxyccb_pd_hw_key_size());
    SDK_ASSERT_RETURN((proxyccb_hwid_ht_ != NULL), false);

    // initialize fte span pd related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_FTE_SPAN_PD)] =
        slab::factory("fte_span_pd", HAL_SLAB_FTE_SPAN_PD,
                      sizeof(hal::pd::pd_fte_span_t), 8,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_FTE_SPAN_PD)] != NULL),
                      false);

    // initialize snake test pd related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_SNAKE_TEST_PD)] =
        slab::factory("snake_test_pd", HAL_SLAB_SNAKE_TEST_PD,
                      sizeof(hal::pd::pd_snake_test_t), 8,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_SNAKE_TEST_PD)] != NULL),
                      false);

    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_SNAKE_TEST_IF_PD)] =
        slab::factory("snake_test_if_pd", HAL_SLAB_SNAKE_TEST_IF_PD,
                      sizeof(hal::pd::pd_snake_test_if_t), 8,
                      true, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_SNAKE_TEST_IF_PD)] != NULL),
                      false);

    dm_tables_ = NULL;
    hash_tcam_tables_ = NULL;
    tcam_tables_ = NULL;
    flow_table_pd_ = NULL;
    met_table_ = NULL;
    acl_table_ = NULL;

    p4plus_rxdma_dm_tables_ = NULL;

    p4plus_txdma_dm_tables_ = NULL;
    cpu_bypass_flowid_ = 0;


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

    // ht
    flow_lkupid_ht_          = NULL;
    tlscb_hwid_ht_           = NULL;
    tcpcb_hwid_ht_           = NULL;
    wring_hwid_ht_           = NULL;
#ifdef __x86_64__
    ipseccb_hwid_ht_         = NULL;
    ipseccb_decrypt_hwid_ht_ = NULL;
    ipsec_sa_hwid_ht_        = NULL;
#endif
    rw_table_ht_             = NULL;
    cpucb_hwid_ht_           = NULL;
    nvme_sesscb_hwid_ht_     = NULL;
    nvme_ns_hwid_ht_         = NULL;
    nvme_sq_hwid_ht_         = NULL;
    nvme_cq_hwid_ht_         = NULL;
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

    flow_lkupid_ht_ ? ht::destroy(flow_lkupid_ht_) : HAL_NOP;
    tlscb_hwid_ht_ ? ht::destroy(tlscb_hwid_ht_) : HAL_NOP;
    tcpcb_hwid_ht_ ? ht::destroy(tcpcb_hwid_ht_) : HAL_NOP;
    wring_hwid_ht_ ? ht::destroy(wring_hwid_ht_) : HAL_NOP;
#ifdef __x86_64__
    ipseccb_hwid_ht_ ? ht::destroy(ipseccb_hwid_ht_) : HAL_NOP;
    ipseccb_decrypt_hwid_ht_ ? ht::destroy(ipseccb_decrypt_hwid_ht_) : HAL_NOP;
    ipsec_sa_hwid_ht_ ? ht::destroy(ipsec_sa_hwid_ht_) : HAL_NOP;
#endif
    cpucb_hwid_ht_ ? ht::destroy(cpucb_hwid_ht_) : HAL_NOP;
    nvme_sesscb_hwid_ht_ ? ht::destroy(nvme_sesscb_hwid_ht_) : HAL_NOP;
    nvme_ns_hwid_ht_ ? ht::destroy(nvme_ns_hwid_ht_) : HAL_NOP;
    nvme_sq_hwid_ht_ ? ht::destroy(nvme_sq_hwid_ht_) : HAL_NOP;
    nvme_cq_hwid_ht_ ? ht::destroy(nvme_cq_hwid_ht_) : HAL_NOP;
    rw_table_ht_ ? ht::destroy(rw_table_ht_) : HAL_NOP;
    rawrcb_hwid_ht_ ? ht::destroy(rawrcb_hwid_ht_) : HAL_NOP;
    rawccb_hwid_ht_ ? ht::destroy(rawccb_hwid_ht_) : HAL_NOP;
    proxyrcb_hwid_ht_ ? ht::destroy(proxyrcb_hwid_ht_) : HAL_NOP;
    proxyccb_hwid_ht_ ? ht::destroy(proxyccb_hwid_ht_) : HAL_NOP;

    if (dm_tables_) {
        for (tid = P4TBL_ID_INDEX_MIN; tid <= P4TBL_ID_INDEX_MAX; tid++) {
            if (dm_tables_[tid-P4TBL_ID_INDEX_MIN]) {
                directmap::destroy(dm_tables_[tid-P4TBL_ID_INDEX_MIN]);
            }
        }
        HAL_FREE(HAL_MEM_ALLOC_PD, dm_tables_);
    }

    if (hash_tcam_tables_) {
        for (tid = P4TBL_ID_HASH_OTCAM_MIN;
             tid <= P4TBL_ID_HASH_OTCAM_MAX; tid++) {
            if (hash_tcam_tables_[tid-P4TBL_ID_HASH_OTCAM_MIN]) {
                sdk_hash::destroy(hash_tcam_tables_[tid-P4TBL_ID_HASH_OTCAM_MIN]);
            }
        }
        HAL_FREE(HAL_MEM_ALLOC_PD, hash_tcam_tables_);
    }

    if (tcam_tables_) {
        for (tid = P4TBL_ID_TCAM_MIN; tid <= P4TBL_ID_TCAM_MAX; tid++) {
            if (tcam_tables_[tid-P4TBL_ID_TCAM_MIN]) {
                tcam::destroy(tcam_tables_[tid-P4TBL_ID_TCAM_MIN]);
            }
        }
        HAL_FREE(HAL_MEM_ALLOC_PD, tcam_tables_);
    }

    if (flow_table_pd_) {
        flow_table_pd::destroy(flow_table_pd_);
    }

    if (met_table_) {
        Met::destroy(met_table_);
    }

    if (acl_table_) {
        delete acl_table_;
    }

    if (p4plus_rxdma_dm_tables_) {
        for (tid = P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN;
             tid <= P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MAX; tid++) {
            if (p4plus_rxdma_dm_tables_[tid - P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN]) {
                directmap::destroy(p4plus_rxdma_dm_tables_[tid -
                                   P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN]);
            }
        }
        HAL_FREE(HAL_MEM_ALLOC_PD, p4plus_rxdma_dm_tables_);
    }

    if (p4plus_txdma_dm_tables_) {
        for (tid = P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN;
             tid <= P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MAX; tid++) {
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
hal_state_pd::factory (void)
{
    hal_state_pd *state;

    state = new hal_state_pd();
    SDK_ASSERT_RETURN((state != NULL), NULL);
    if (state->init() == false) {
        delete state;
        return NULL;
    }
    return state;
}

hal_ret_t
pd_get_slab (pd_func_args_t *pd_func_args)
{
    pd_get_slab_args_t *args = pd_func_args->pd_get_slab;
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
    p4pd_error_t               p4pd_ret;
    p4pd_table_properties_t    tinfo, ctinfo;
    p4pd_cfg_t                 p4pd_cfg = {
        .table_map_cfg_file  = "iris/capri_p4_table_map.json",
        .p4pd_pgm_name       = "iris",
        .p4pd_rxdma_pgm_name = "p4plus",
        .p4pd_txdma_pgm_name = "p4plus",
        .cfg_path            = args->cfg_path,
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
    SDK_ASSERT(dm_tables_ != NULL);

    // make sure there is one flow table only
    SDK_ASSERT((P4TBL_ID_HASH_MAX - P4TBL_ID_HASH_MIN + 1) == 2);

    hash_tcam_tables_ =
        (sdk_hash **)HAL_CALLOC(HAL_MEM_ALLOC_PD,
                         sizeof(sdk_hash *) *
                         (P4TBL_ID_HASH_OTCAM_MAX - P4TBL_ID_HASH_OTCAM_MIN + 1));
    SDK_ASSERT(hash_tcam_tables_ != NULL);

    tcam_tables_ =
        (tcam **)HAL_CALLOC(HAL_MEM_ALLOC_PD,
                            sizeof(tcam *) *
                            (P4TBL_ID_TCAM_MAX - P4TBL_ID_TCAM_MIN + 1));
    SDK_ASSERT(tcam_tables_ != NULL);

    met_table_ = Met::factory(P4_REPL_TABLE_NAME, P4_REPL_TABLE_ID,
                              P4_REPL_TABLE_DEPTH,
                              CAPRI_REPL_NUM_P4_ENTRIES_PER_NODE,
                              P4_REPL_ENTRY_WIDTH,
                              HAL_MEM_ALLOC_MET,
                              table_health_monitor,
                              repl_entry_data_to_str);
    SDK_ASSERT(met_table_ != NULL);

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
                              ENTRY_TRACE_EN, table_health_monitor);
            SDK_ASSERT(hash_tcam_tables_[tid - P4TBL_ID_HASH_OTCAM_MIN] != NULL);
            break;

        case P4_TBL_TYPE_TCAM:
            if (tid == P4TBL_ID_NACL) {
                // ACL tcam table with priority 0 as highest and no duplicate
                // priorities
                acl_table_ = acl_tcam::factory(tinfo.tablename, tid, tinfo.tabledepth,
                                               tinfo.key_struct_size,
                                               tinfo.actiondata_struct_size,
                                               false, true);
                SDK_ASSERT(acl_table_ != NULL);
#if 0
            } else if ((tid == P4TBL_ID_DDOS_SRC_VF) ||
                       (tid == P4TBL_ID_DDOS_SRC_DST) ||
                       (tid == P4TBL_ID_DDOS_SERVICE)) {
                // Allow dup entries must be set to true for ddos tcam tables
                if (!tinfo.is_oflow_table) {
                    tcam_tables_[tid - P4TBL_ID_TCAM_MIN] =
                        tcam::factory(tinfo.tablename, tid, tinfo.tabledepth,
                                      tinfo.key_struct_size, tinfo.actiondata_struct_size, true,
                                      ENTRY_TRACE_EN, table_health_monitor);
                    SDK_ASSERT(tcam_tables_[tid - P4TBL_ID_TCAM_MIN] != NULL);
                }
#endif
            } else {
                if (!tinfo.is_oflow_table) {
                    tcam_tables_[tid - P4TBL_ID_TCAM_MIN] =
                        tcam::factory(tinfo.tablename, tid, tinfo.tabledepth,
                                      tinfo.key_struct_size, tinfo.actiondata_struct_size, false,
                                      ENTRY_TRACE_EN, table_health_monitor);
                    SDK_ASSERT(tcam_tables_[tid - P4TBL_ID_TCAM_MIN] != NULL);
                }
            }

            break;

        case P4_TBL_TYPE_INDEX:
            if (tid == P4TBL_ID_TWICE_NAT) {
                dm_tables_[tid - P4TBL_ID_INDEX_MIN] =
                    directmap::factory(tinfo.tablename, tid, tinfo.tabledepth,
                                       tinfo.actiondata_struct_size, true,
                                       ENTRY_TRACE_EN, table_health_monitor);
            } else {
                bool trace_en = true;
                if (tid == P4TBL_ID_SESSION_STATE || tid == P4TBL_ID_FLOW_INFO ||
                    tid == P4TBL_ID_FLOW_STATS) {
                    sdk_table_factory_params_t params;

                    bzero(&params, sizeof(sdk_table_factory_params_t));
                    params.entry_trace_en = false;
                    params.table_id = tid;
                    dm_tables_[tid - P4TBL_ID_INDEX_MIN] =
                        (directmap*)sldirectmap::factory(&params);
                    // Set read/write thru mode for Flow Info, Session State & Session Stats tables
                    p4pd_ret = p4pd_table_properties_set_write_mode(tid, P4_TBL_WRITE_MODE_WRITE_THRU);
                    if (p4pd_ret != P4PD_SUCCESS) {
                        HAL_TRACE_ERR("Failed to set table write thru mode {} for tid {}", P4_TBL_WRITE_MODE_WRITE_THRU, tid);
                        SDK_ASSERT(0);
                    }
                    p4pd_ret = p4pd_table_properties_set_read_thru_mode(tid, true);
                    if (p4pd_ret != P4PD_SUCCESS) {
                        HAL_TRACE_ERR("Failed to set table read thru mode to true for tid {}", tid);
                        SDK_ASSERT(0);
                    }
                } else {
                    dm_tables_[tid - P4TBL_ID_INDEX_MIN] =
                        directmap::factory(tinfo.tablename, tid, tinfo.tabledepth,
                                           tinfo.actiondata_struct_size, false,
                                           trace_en, table_health_monitor);
                }
            }
            SDK_ASSERT(dm_tables_[tid - P4TBL_ID_INDEX_MIN] != NULL);
            break;

        case P4_TBL_TYPE_HASH:
            if (tid == P4TBL_ID_FLOW_HASH) {
                p4pd_ret = p4pd_table_properties_set_write_mode(tid, P4_TBL_WRITE_MODE_WRITE_THRU);

                if (p4pd_ret != P4PD_SUCCESS) {
                    HAL_TRACE_ERR("Failed to set table write thru mode {} for tid {}", P4_TBL_WRITE_MODE_WRITE_THRU, tid);
                    SDK_ASSERT(0);
                }

                p4pd_ret = p4pd_table_properties_set_read_thru_mode(tid, true);

                if (p4pd_ret != P4PD_SUCCESS) {
                    HAL_TRACE_ERR("Failed to set table read thru mode to true for tid {}", tid);
                    SDK_ASSERT(0);
                }
            }

            if (tid == P4TBL_ID_FLOW_HASH_OVERFLOW) {
                break;
            }
            SDK_ASSERT(tid == P4TBL_ID_FLOW_HASH);
            break;

        case P4_TBL_TYPE_MPU:
        default:
            break;
        }
    }

    return ret;
}

hal_ret_t
hal_state_pd::init_flow_table(void)
{
    flow_table_pd_ = flow_table_pd::factory();
    SDK_ASSERT(flow_table_pd_ != NULL);

    return HAL_RET_OK;
}

hal_ret_t
hal_state_pd::p4plus_rxdma_init_tables(pd_mem_init_args_t *args)
{
    uint32_t                   tid;
    hal_ret_t                  ret = HAL_RET_OK;
    p4pd_table_properties_t    tinfo, ctinfo;
    p4pd_error_t               rc;
    p4pd_cfg_t                 p4pd_cfg;

    pipeline_cfg_init(NULL, &p4pd_cfg, NULL);
    p4pd_cfg.cfg_path = args->cfg_path;

    memset(&tinfo, 0, sizeof(tinfo));
    memset(&ctinfo, 0, sizeof(ctinfo));

    // parse the NCC generated table info file for p4+ tables
    rc = p4pluspd_rxdma_init(&p4pd_cfg);
    SDK_ASSERT(rc == P4PD_SUCCESS);

    // start instantiating tables based on the parsed information
    p4plus_rxdma_dm_tables_ =
        (directmap **)HAL_CALLOC(HAL_MEM_ALLOC_PD,
                                 sizeof(directmap *) *
                                 (P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MAX -
                                  P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN + 1));
    SDK_ASSERT(p4plus_rxdma_dm_tables_ != NULL);

    // TODO:
    // 1. take care of instantiating flow_table_, acl_table_ and met_table_
    // 2. When tables are instantiated proper names are not passed today,
    // waiting for an API from Mahesh that gives table name given table id

    for (tid = P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMIN;
         tid < P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMAX; tid++) {
        rc = p4pluspd_rxdma_table_properties_get(tid, &tinfo);
        SDK_ASSERT(rc == P4PD_SUCCESS);

        switch (tinfo.table_type) {
        case P4_TBL_TYPE_INDEX:
            if (tinfo.tabledepth) {
                p4plus_rxdma_dm_tables_[tid - P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN] =
                    directmap::factory(tinfo.tablename, tid, tinfo.tabledepth, tinfo.actiondata_struct_size,
                                       false, ENTRY_TRACE_EN, table_health_monitor);
                SDK_ASSERT(p4plus_rxdma_dm_tables_[tid - P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN] != NULL);
            }
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
    p4pd_cfg_t                 p4pd_cfg;

    pipeline_cfg_init(NULL, NULL, &p4pd_cfg);
    p4pd_cfg.cfg_path = args->cfg_path;

    memset(&tinfo, 0, sizeof(tinfo));
    memset(&ctinfo, 0, sizeof(ctinfo));

    // parse the NCC generated table info file for p4+ tables
    rc = p4pluspd_txdma_init(&p4pd_cfg);
    SDK_ASSERT(rc == P4PD_SUCCESS);

    // start instantiating tables based on the parsed information
    p4plus_txdma_dm_tables_ =
        (directmap **)HAL_CALLOC(HAL_MEM_ALLOC_PD,
                                 sizeof(directmap *) *
                                 (P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MAX -
                                  P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN + 1));
    SDK_ASSERT(p4plus_txdma_dm_tables_ != NULL);

    // TODO:
    // 1. take care of instantiating flow_table_, acl_table_ and met_table_
    // 2. When tables are instantiated proper names are not passed today,
    // waiting for an API from Mahesh that gives table name given table id

    for (tid = P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMIN;
         tid < P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMAX; tid++) {
        rc = p4pluspd_txdma_table_properties_get(tid, &tinfo);
        SDK_ASSERT(rc == P4PD_SUCCESS);

        switch (tinfo.table_type) {
        case P4_TBL_TYPE_INDEX:
            if (tinfo.tabledepth) {
                p4plus_txdma_dm_tables_[tid - P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN] =
                    directmap::factory(tinfo.tablename, tid, tinfo.tabledepth, tinfo.actiondata_struct_size,
                                       false, ENTRY_TRACE_EN, table_health_monitor);
                SDK_ASSERT(p4plus_txdma_dm_tables_[tid - P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN] != NULL);
            }
            break;

        case P4_TBL_TYPE_MPU:
        default:
            break;
        }
    }

    return ret;
}

static hal_ret_t
hal_pd_lif_stats_region_init (void)
{
    p4pd_table_properties_t     tbl_ctx;
    p4pd_error_t                rc;
    uint64_t                    stats_base_addr;

    stats_base_addr = get_mem_addr(CAPRI_HBM_REG_LIF_STATS);
    // subtract 2G (saves ASM instructions)
    stats_base_addr -= ((uint64_t)1 << 31);

    rc = p4pd_table_properties_get(P4TBL_ID_INGRESS_TX_STATS, &tbl_ctx);
    SDK_ASSERT(rc == P4PD_SUCCESS);
    capri_table_constant_write(stats_base_addr,
                               tbl_ctx.stage, tbl_ctx.stage_tableid,
                               (tbl_ctx.gress == P4_GRESS_INGRESS));

    rc = p4pd_table_properties_get(P4TBL_ID_TX_STATS, &tbl_ctx);
    SDK_ASSERT(rc == P4PD_SUCCESS);
    capri_table_constant_write(stats_base_addr,
                               tbl_ctx.stage, tbl_ctx.stage_tableid,
                               (tbl_ctx.gress == P4_GRESS_INGRESS));

    return HAL_RET_OK;
}

static hal_ret_t
hal_pd_hwerr_stats_region_init (void)
{
    p4pd_table_properties_t     tbl_ctx;
    p4pd_error_t                rc;
    uint64_t                    stats_base_addr;

    stats_base_addr = get_mem_addr(CAPRI_HBM_REG_P4_HWERR_STATS);
    // subtract 2G (saves ASM instructions)
    stats_base_addr -= ((uint64_t)1 << 31);

    rc = p4pd_table_properties_get(P4TBL_ID_DROP_STATS, &tbl_ctx);
    SDK_ASSERT(rc == P4PD_SUCCESS);
    capri_table_constant_write(stats_base_addr,
                               tbl_ctx.stage, tbl_ctx.stage_tableid,
                               (tbl_ctx.gress == P4_GRESS_INGRESS));

    stats_base_addr += 512;
    rc = p4pd_table_properties_get(P4TBL_ID_EGRESS_DROP_STATS, &tbl_ctx);
    SDK_ASSERT(rc == P4PD_SUCCESS);
    capri_table_constant_write(stats_base_addr,
                               tbl_ctx.stage, tbl_ctx.stage_tableid,
                               (tbl_ctx.gress == P4_GRESS_INGRESS));

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// one time memory related initialization for HAL
//------------------------------------------------------------------------------
hal_ret_t
pd_mem_init (pd_func_args_t *pd_func_args)
{
    hal_ret_t   ret = HAL_RET_OK;
    pd_mem_init_args_t *args = pd_func_args->pd_mem_init;

    g_hal_state_pd = hal_state_pd::factory();
    SDK_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

    HAL_TRACE_DEBUG("Initializing p4 asic lib tables ...");
    ret = g_hal_state_pd->init_tables(args);
    if (ret != HAL_RET_OK) {
        delete g_hal_state_pd;
    }
    g_hal_state_pd->set_hal_cfg(args->hal_cfg);

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
pd_mem_init_phase2 (pd_func_args_t *pd_func_args)
{
    p4pd_cfg_t    p4pd_cfg;

    pd_mem_init_phase2_args_t           *ph2_args;
    hal::hal_cfg_t                      *hal_cfg;

    pipeline_cfg_init(&p4pd_cfg, NULL, NULL);

    ph2_args = pd_func_args->pd_mem_init_phase2;
    hal_cfg = ph2_args->hal_cfg;

    SDK_ASSERT(sdk::asic::pd::asicpd_p4plus_table_mpu_base_init(&p4pd_cfg) == SDK_RET_OK);
    SDK_ASSERT(asicpd_stats_region_init(g_stats_region_arr,
                                        g_stats_region_arrlen) == HAL_RET_OK);
    SDK_ASSERT(hal_pd_lif_stats_region_init() == HAL_RET_OK);
    SDK_ASSERT(hal_pd_hwerr_stats_region_init() == HAL_RET_OK);
    SDK_ASSERT(asicpd_toeplitz_init() == HAL_RET_OK);
    SDK_ASSERT(asicpd_p4plus_table_init(hal_cfg) == HAL_RET_OK);
    SDK_ASSERT(sdk::asic::pd::asicpd_p4plus_recirc_init() == SDK_RET_OK);
    // Following routines must be called after capri asic init
    SDK_ASSERT(sdk::asic::pd::asicpd_table_mpu_base_init(&p4pd_cfg) == SDK_RET_OK);
    SDK_ASSERT(sdk::asic::pd::asicpd_program_table_mpu_pc() == SDK_RET_OK);
    SDK_ASSERT(sdk::asic::pd::asicpd_deparser_init() == SDK_RET_OK);
    SDK_ASSERT(sdk::asic::pd::asicpd_program_hbm_table_base_addr() == SDK_RET_OK);
    g_hal_state_pd->init_flow_table();

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// program default entries in tables
//------------------------------------------------------------------------------
hal_ret_t
pd_pgm_def_entries (pd_func_args_t *pd_func_args)
{
    hal_ret_t   ret = HAL_RET_OK;
    p4pd_def_cfg_t  p4pd_def_cfg;
    pd_pgm_def_entries_args_t *args = pd_func_args->pd_pgm_def_entries;

    HAL_TRACE_DEBUG("Programming table default entries ...");
    p4pd_def_cfg.admin_cos = qos_class_get_admin_cos();
    p4pd_def_cfg.hal_cfg = args->hal_cfg;
    ret = p4pd_table_defaults_init(&p4pd_def_cfg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program default entries, err : {}", ret);
    }

    return ret;
}

//------------------------------------------------------------------------------
// program default entries for P4Plus tables
//------------------------------------------------------------------------------
hal_ret_t
pd_pgm_def_p4plus_entries (pd_func_args_t *pd_func_args)
{
    hal_ret_t   ret = HAL_RET_OK;
    // pd_pgm_def_p4plus_entries_args_t *args = pd_func_args->pd_pgm_def_p4plus_entries;

    HAL_TRACE_DEBUG("Programming p4plus default entries ...");
    ret = wring_pd_init_global_rings();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program default rings, err : {}", ret);
        return ret;
    }

    ret = crypto_pd_init();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to intialize Capri Barco err : {}", ret);
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

    case HAL_SLAB_TUNNELIF_PD:
        g_hal_state_pd->tunnelif_pd_slab()->free(elem);
        break;

    case HAL_SLAB_CPUIF_PD:
        g_hal_state_pd->cpuif_pd_slab()->free(elem);
        break;

    case HAL_SLAB_DOS_POLICY_PD:
        g_hal_state_pd->dos_pd_slab()->free(elem);
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

    case HAL_SLAB_NVME_GLOBAL_PD:
        g_hal_state_pd->nvme_global_slab()->free(elem);
        break;

    case HAL_SLAB_NVME_SESSCB_PD:
        g_hal_state_pd->nvme_sesscb_slab()->free(elem);
        break;

    case HAL_SLAB_NVME_NS_PD:
        g_hal_state_pd->nvme_ns_slab()->free(elem);
        break;

    case HAL_SLAB_NVME_SQ_PD:
        g_hal_state_pd->nvme_sq_slab()->free(elem);
        break;

    case HAL_SLAB_NVME_CQ_PD:
        g_hal_state_pd->nvme_cq_slab()->free(elem);
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

    case HAL_SLAB_CPUCB_PD:
        g_hal_state_pd->cpucb_slab()->free(elem);
        break;

#ifdef __x86_64__
    case HAL_SLAB_IPSECCB_PD:
        g_hal_state_pd->ipseccb_slab()->free(elem);
        break;

    case HAL_SLAB_IPSECCB_DECRYPT_PD:
        g_hal_state_pd->ipseccb_decrypt_slab()->free(elem);
        break;

    case HAL_SLAB_L4LB_PD:
        g_hal_state_pd->l4lb_pd_slab()->free(elem);
        break;

    case HAL_SLAB_IPSEC_SA_PD:
        g_hal_state_pd->ipsec_sa_slab()->free(elem);
        break;

#endif

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

    case HAL_SLAB_FTE_SPAN_PD:
        g_hal_state_pd->fte_span_slab()->free(elem);
        break;

    case HAL_SLAB_SNAKE_TEST_PD:
        g_hal_state_pd->snake_test_slab()->free(elem);
        break;

    case HAL_SLAB_SNAKE_TEST_IF_PD:
        g_hal_state_pd->snake_test_if_slab()->free(elem);
        break;

    default:
        HAL_TRACE_ERR("Unknown slab id {}", slab_id);
        SDK_ASSERT(FALSE);
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
        HAL_TRACE_ERR("Unexpected slab id {}", slab_id);
        ret = HAL_RET_INVALID_ARG;
    }
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to release elem {} to slab id {}", elem, slab_id);
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

    if (g_delay_delete && sdk::lib::periodic_thread_is_running()) {
        timer_ctxt =
            sdk::lib::timer_schedule(slab_id,
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

hal_ret_t
hal_pd_stats_addr_get (int tblid, uint32_t index,
                       mem_addr_t *stats_addr_p)
{
    return asicpd_stats_addr_get(tblid, index,
                                 g_stats_region_arr, g_stats_region_arrlen,
                                 stats_addr_p);
}

}    // namespace pd
}    // namespace hal


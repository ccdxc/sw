// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_STATE_PD_HPP__
#define __HAL_STATE_PD_HPP__

#include "lib/indexer/indexer.hpp"
#include "lib/slab/slab.hpp"
#include "lib/ht/ht.hpp"
#include "lib/table/directmap/directmap.hpp"
#include "lib/table/sldirectmap/sldirectmap.hpp"
#include "lib/table/hash/hash.hpp"
#include "lib/table/tcam/tcam.hpp"
#include "nic/hal/pd/utils/met/met.hpp"
#include "nic/hal/pd/utils/acl_tcam/acl_tcam.hpp"
#include "gen/p4gen/p4/include/p4pd_table.h"
#include "gen/p4gen/common_txdma_actions/include/common_txdma_actions_p4pd_table.h"
#include "gen/p4gen/common_rxdma_actions/include/common_rxdma_actions_p4pd_table.h"
#include "gen/p4gen/p4/include/p4pd.h"
#include "gen/p4gen/common_txdma_actions/include/common_txdma_actions_p4pd.h"
#include "gen/p4gen/common_rxdma_actions/include/common_rxdma_actions_p4pd.h"
#include "lib/bm_allocator/bm_allocator.hpp"
#include "platform/capri/capri_tm_rw.hpp"
#include "nic/hal/pd/pd_api.hpp"
#include "nic/hal/pd/iris/flow/flow_table_pd.hpp"

using sdk::lib::BMAllocator;
using sdk::lib::indexer;
using sdk::lib::slab;
using sdk::lib::ht;
using sdk::table::tcam;
using sdk::table::directmap;
using hal::pd::utils::Met;
using hal::pd::utils::acl_tcam;
using hal::pd::flow_table_pd;

namespace hal {
namespace pd {

extern class hal_state_pd    *g_hal_state_pd;

// LIF HW ID Space for SB LIFs, Uplink Ifs/PCs
#define HAL_MAX_HW_LIFS         1025
#define HAL_MAX_UPLINK_IFS      16
#define HAL_MAX_UPLINK_IF_PCS   32      // Both Uplink IFs and PCs combined

#define HAL_RW_TABLE_SIZE               4096
#define HAL_TUNNEL_RW_TABLE_SIZE        1024

#define HAL_MAX_HW_ACLS                 512

#define HAL_PD_SLAB_ID(slab_id) slab_id - HAL_SLAB_PD_MIN

typedef enum hal_clock_delta_op_s {
    HAL_CLOCK_DELTA_OP_ADD                = 0,
    HAL_CLOCK_DELTA_OP_SUBTRACT           = 1,
} hal_clock_delta_op_t;

//-----------------------------------------------------------------------------
// class hal_state_pd
//
//  - slab memory instances for all pd objects.
//  - indexers needed for pd
//-----------------------------------------------------------------------------
class hal_state_pd {
public:
    static hal_state_pd *factory(void);
    ~hal_state_pd();

    slab *get_slab(hal_slab_t slab_id);

    // get APIs for global variables
    uint32_t rwr_tbl_decap_vlan_idx(void) const {
        return rewr_tbl_decap_vlan_idx_; }
    void set_rwr_tbl_decap_vlan_idx(uint32_t idx) {
        rewr_tbl_decap_vlan_idx_ = idx; }

    uint32_t tnnl_rwr_tbl_encap_vlan_idx(void) const {
        return tnnl_rewr_tbl_enc_vlan_idx_; }
    void set_tnnl_rwr_tbl_encap_vlan_idx(uint32_t idx) {
        tnnl_rewr_tbl_enc_vlan_idx_ = idx; }


    // get slabs
    slab *vrf_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_VRF_PD)]; }
    slab *l2seg_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_L2SEG_PD)]; }
    slab *mc_entry_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_MC_ENTRY_PD)]; }
    slab *lif_pd_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_LIF_PD)]; }
    slab *uplinkif_pd_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_UPLINKIF_PD)]; }
    slab *uplinkpc_pd_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_UPLINKPC_PD)]; }
    slab *enicif_pd_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_ENICIF_PD)]; }
    slab *if_l2seg_entry_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_IF_L2SEG_PD)]; }
    slab *tunnelif_pd_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_TUNNELIF_PD)]; }
    slab *cpuif_pd_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_CPUIF_PD)]; }
    slab *dos_pd_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_DOS_POLICY_PD)]; }
    slab *nwsec_pd_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_SECURITY_PROFILE_PD)]; }
    slab *ep_pd_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_EP_PD)]; }
    slab *ep_pd_ip_entry_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_EP_IP_ENTRY_PD)]; }
    slab *session_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_SESSION_PD)]; }
    slab *tlscb_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_TLSCB_PD)]; }
    slab *tcpcb_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_TCPCB_PD)]; }
    slab *nvme_global_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_NVME_GLOBAL_PD)]; }
    slab *nvme_sesscb_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_NVME_SESSCB_PD)]; }
    slab *nvme_ns_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_NVME_NS_PD)]; }
    slab *nvme_sq_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_NVME_SQ_PD)]; }
    slab *nvme_cq_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_NVME_CQ_PD)]; }
    slab *qos_class_pd_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_QOS_CLASS_PD)]; }
    slab *acl_pd_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_ACL_PD)]; }
    slab *wring_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_WRING_PD)]; }
    slab *ipseccb_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_IPSECCB_PD)]; }
    slab *ipseccb_decrypt_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_IPSECCB_DECRYPT_PD)]; }
    slab *ipsec_sa_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_IPSEC_SA_PD)]; }
    slab *l4lb_pd_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_L4LB_PD)]; }
    slab *rw_entry_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_RW_PD)]; }
    slab *tnnl_rw_entry_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_TUNNEL_RW_PD)]; }
    slab *cpucb_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_CPUCB_PD)]; }
    slab *cpupkt_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_CPUPKT_PD)]; }
    slab *tcp_rings_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_TCP_RINGS_PD)]; }
    slab *rawrcb_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_RAWRCB_PD)]; }
    slab *rawccb_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_RAWCCB_PD)]; }
    slab *proxyrcb_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_PROXYRCB_PD)]; }
    slab *proxyccb_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_PROXYCCB_PD)]; }
    slab *cpupkt_qinst_info_slab(void) const {return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_CPUPKT_QINST_INFO_PD)]; }
    slab *copp_pd_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_COPP_PD)]; }
    slab *app_redir_if_pd_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_APP_REDIR_IF_PD)]; }
    slab *fte_span_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_FTE_SPAN_PD)]; }
    slab *snake_test_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_SNAKE_TEST_PD)]; }
    slab *snake_test_if_slab(void) const { return slabs_[HAL_PD_SLAB_ID(HAL_SLAB_SNAKE_TEST_IF_PD)]; }

    // get hts
    ht *flow_lkupid_ht(void) const { return flow_lkupid_ht_; }
    ht *tlscb_hwid_ht(void) const { return tlscb_hwid_ht_; }
    ht *tcpcb_hwid_ht(void) const { return tcpcb_hwid_ht_; }
    ht *nvme_sesscb_hwid_ht(void) const { return nvme_sesscb_hwid_ht_; }
    ht *nvme_ns_hwid_ht(void) const { return nvme_ns_hwid_ht_; }
    ht *nvme_sq_hwid_ht(void) const { return nvme_sq_hwid_ht_; }
    ht *nvme_cq_hwid_ht(void) const { return nvme_cq_hwid_ht_; }
    ht *wring_hwid_ht(void) const { return wring_hwid_ht_; }
    ht *ipseccb_hwid_ht(void) const { return ipseccb_hwid_ht_; }
    ht *ipseccb_decrypt_hwid_ht(void) const { return ipseccb_decrypt_hwid_ht_; }
    ht *ipsec_sa_hwid_ht(void) const { return ipsec_sa_hwid_ht_; }
    ht *cpucb_hwid_ht(void) const { return cpucb_hwid_ht_; }
    ht *rawrcb_hwid_ht(void) const { return rawrcb_hwid_ht_; }
    ht *rawccb_hwid_ht(void) const { return rawccb_hwid_ht_; }
    ht *proxyrcb_hwid_ht(void) const { return proxyrcb_hwid_ht_; }
    ht *proxyccb_hwid_ht(void) const { return proxyccb_hwid_ht_; }
    ht *rw_table_ht(void) const { return rw_table_ht_; }
    ht *tnnl_rw_table_ht(void) const { return tnnl_rw_table_ht_; }

    // get indexers
    indexer *vrf_hwid_idxr(void) const { return vrf_hwid_idxr_; }
    indexer *nwsec_profile_hwid_idxr(void) const { return nwsec_profile_hwid_idxr_; }
    indexer *l2seg_cpu_idxr(void) const { return l2seg_cpu_idxr_; }
    indexer *lport_idxr(void) const { return lport_idxr_; }
    indexer *lif_hwid_idxr(void) const { return lif_hwid_idxr_; }
    indexer *uplinkifpc_hwid_idxr(void) const { return uplinkifpc_idxr_; }
    indexer *qos_txdma_iq_idxr(void) { return qos_txdma_iq_idxr_; }
    indexer *qos_uplink_iq_idxr(void) { return qos_uplink_iq_idxr_; }
    indexer *qos_common_oq_idxr(void) { return qos_common_oq_idxr_; }
    indexer *qos_rxdma_oq_idxr(void) { return qos_rxdma_oq_idxr_; }
    indexer *rw_tbl_idxr(void) { return rw_tbl_idxr_; }
    indexer *tnnl_rw_tbl_idxr(void) { return tnnl_rw_tbl_idxr_; }

    hal_ret_t init_tables(pd_mem_init_args_t *args);
    hal_ret_t p4plus_rxdma_init_tables(pd_mem_init_args_t *args);
    hal_ret_t p4plus_txdma_init_tables(pd_mem_init_args_t *args);

    directmap *dm_table(uint32_t tid) const {
        if ((tid < P4TBL_ID_INDEX_MIN) || (tid > P4TBL_ID_INDEX_MAX)) {
            return NULL;
        }
        return dm_tables_[tid - P4TBL_ID_INDEX_MIN];
    }

    sdk_hash *hash_tcam_table(uint32_t tid) const {
        if ((tid < P4TBL_ID_HASH_OTCAM_MIN) ||
            (tid > P4TBL_ID_HASH_OTCAM_MAX)) {
            return NULL;
        }
        return hash_tcam_tables_[tid - P4TBL_ID_HASH_OTCAM_MIN];
    }

    tcam *tcam_table(uint32_t tid) const {
        if ((tid < P4TBL_ID_TCAM_MIN) || (tid > P4TBL_ID_TCAM_MAX)) {
            return NULL;
        }
        return tcam_tables_[tid - P4TBL_ID_TCAM_MIN];
    }

    flow_table_pd *flow_table_pd_get(void) const { return flow_table_pd_; }

    Met *met_table(void) const { return met_table_; }

    acl_tcam *acl_table(void) const { return acl_table_; }

    directmap *p4plus_rxdma_dm_table(uint32_t tid) const {
        if ((tid < P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN) ||
            (tid > P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MAX)) {
            return NULL;
        }
        return p4plus_rxdma_dm_tables_[tid - P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN];
    }

    directmap *p4plus_txdma_dm_table(uint32_t tid) const {
        if ((tid < P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN) ||
            (tid > P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MAX)) {
            return NULL;
        }
        return p4plus_txdma_dm_tables_[tid - P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN];
    }

    void set_clock_delta(uint64_t delta) { clock_delta_ = delta; }
    uint64_t clock_delta(void) { return clock_delta_; }

    void set_clock_delta_op(hal_clock_delta_op_t op) { clock_delta_op_ = op; }
    hal_clock_delta_op_t clock_delta_op(void) { return clock_delta_op_; }

    void set_cpu_bypass_flowid(uint32_t flowid) { cpu_bypass_flowid_ = flowid; }
    uint32_t cpu_bypass_flowid(void) { return cpu_bypass_flowid_; }

    void set_hal_cfg(hal_cfg_t *hal_cfg) { hal_cfg_ = hal_cfg; }
    hal_cfg_t *hal_cfg(void) { return hal_cfg_; }

private:
    bool init(void);
    hal_state_pd();

    // global variables
    struct {
        uint32_t  rewr_tbl_decap_vlan_idx_;
        uint32_t  tnnl_rewr_tbl_enc_vlan_idx_;
    } __PACK__;

    // vrf related state
    struct {
        indexer    *vrf_hwid_idxr_;
    } __PACK__;

    struct {
        indexer    *nwsec_profile_hwid_idxr_;
    } __PACK__;

    // l2seg related state
    struct {
        ht         *flow_lkupid_ht_;
        indexer    *l2seg_cpu_idxr_;
    } __PACK__;

    // mc entry related state
    struct {
    } __PACK__;

    // Lport Indexer
    struct {
        indexer    *lport_idxr_;
    } __PACK__;

    // LIF related state
    struct {
        indexer    *lif_hwid_idxr_;         // Used even by Uplink IF/PCs
    } __PACK__;

    // Uplink IF related state
    struct {
        indexer    *uplinkifpc_idxr_;       // User for both Uplink IF/PCs
    } __PACK__;

    // tlscb related state
    struct {
        ht         *tlscb_hwid_ht_;
    } __PACK__;

    // tcpcb related state
    struct {
        ht         *tcpcb_hwid_ht_;
    } __PACK__;

    // nvme_sesscb related state
    struct {
        ht         *nvme_sesscb_hwid_ht_;
    } __PACK__;

    // nvme_ns related state
    struct {
        ht         *nvme_ns_hwid_ht_;
    } __PACK__;

    // nvme_sq related state
    struct {
        ht         *nvme_sq_hwid_ht_;
    } __PACK__;

    // nvme_cq related state
    struct {
        ht         *nvme_cq_hwid_ht_;
    } __PACK__;

    // Qos related state
    struct {
        // Array of indexers for each of the tm port types - uplink, p4, dma
        indexer    *qos_txdma_iq_idxr_;
        indexer    *qos_uplink_iq_idxr_;
        indexer    *qos_common_oq_idxr_;
        indexer    *qos_rxdma_oq_idxr_;
    } __PACK__;

    // wring related state
    struct {
        ht         *wring_hwid_ht_;
    } __PACK__;

    // ipseccb related state
    struct {
        ht         *ipseccb_hwid_ht_;
        ht         *ipsec_sa_hwid_ht_;
        ht         *ipseccb_decrypt_hwid_ht_;
    } __PACK__;

    // rw table management
    struct {
        ht        *rw_table_ht_;
        indexer   *rw_tbl_idxr_;
    } __PACK__;

    // tnnl rw table management
    struct {
        ht        *tnnl_rw_table_ht_;
        indexer   *tnnl_rw_tbl_idxr_;
    } __PACK__;

    // cpucb related state
    struct {
        ht         *cpucb_hwid_ht_;
    } __PACK__;

    // Raw Redirect CB related state
    struct {
        ht         *rawrcb_hwid_ht_;
    } __PACK__;

    // Proxy Redirect CB related state
    struct {
        ht         *proxyrcb_hwid_ht_;
    } __PACK__;

    // Raw Chain CB related state
    struct {
        ht         *rawccb_hwid_ht_;
    } __PACK__;

    // Proxy Chain CB related state
    struct {
        ht         *proxyccb_hwid_ht_;
    } __PACK__;

    // Bypass Flow info
    struct {
        uint32_t    cpu_bypass_flowid_;
    } __PACK__;

    slab                    *slabs_[HAL_SLAB_PD_MAX - HAL_SLAB_PD_MIN + 1];
    directmap               **dm_tables_;
    sdk_hash                **hash_tcam_tables_;
    tcam                    **tcam_tables_;
    flow_table_pd           *flow_table_pd_; 
    Met                     *met_table_;
    acl_tcam                *acl_table_;
    directmap               **p4plus_rxdma_dm_tables_;
    directmap               **p4plus_txdma_dm_tables_;
    uint64_t                clock_delta_;    // hw sw clock delta in nanoseconds
    hal_clock_delta_op_t    clock_delta_op_; // hw sw clock delta op
    hal_cfg_t               *hal_cfg_;
};

hal_ret_t delay_delete_to_slab(hal_slab_t slab_id, void *elem);

hal_ret_t hal_pd_stats_addr_get(int tblid, uint32_t index,
                                mem_addr_t *stats_addr_p);
} // namespace pd
} // namespace hal

#endif    // __HAL_STATE_PD_HPP__


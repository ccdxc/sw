#ifndef __HAL_STATE_PD_HPP__
#define __HAL_STATE_PD_HPP__

#include "sdk/indexer.hpp"
#include "sdk/slab.hpp"
#include "sdk/ht.hpp"
#include "sdk/directmap.hpp"
#include "sdk/hash.hpp"
#include "sdk/tcam.hpp"
#include "nic/hal/pd/utils/flow/flow.hpp"
#include "nic/hal/pd/utils/met/met.hpp"
#include "nic/hal/pd/utils/acl_tcam/acl_tcam.hpp"
#include "nic/gen/iris/include/p4pd.h"
#include "nic/gen/common_txdma_actions/include/common_txdma_actions_p4pd.h"
#include "nic/gen/common_rxdma_actions/include/common_rxdma_actions_p4pd.h"
#include "nic/utils/bm_allocator/bm_allocator.hpp"
#include "nic/hal/pd/capri/capri_tm_rw.hpp"

using hal::BMAllocator;
using sdk::lib::indexer;
using sdk::lib::slab;
using sdk::lib::ht;
using sdk::table::tcam;
using sdk::table::directmap;
using hal::pd::utils::Flow;
using hal::pd::utils::Met;
using hal::pd::utils::acl_tcam;

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

    // get APIs for vrf related state
    slab *vrf_slab(void) const { return vrf_slab_; }
    indexer *vrf_hwid_idxr(void) const { return vrf_hwid_idxr_; }

    // get APIs for security related state
    indexer *nwsec_profile_hwid_idxr(void) const { return nwsec_profile_hwid_idxr_; }

    // get APIs for L2 segment related state
    slab *l2seg_slab(void) const { return l2seg_slab_; }
    ht *flow_lkupid_ht(void) const { return flow_lkupid_ht_; }
    indexer *l2seg_cpu_idxr(void) const { return l2seg_cpu_idxr_; }

    // get APIs for mc entry related state
    slab *mc_entry_slab(void) const { return mc_entry_slab_; }

    // get API for lport indexer
    indexer *lport_idxr(void) const { return lport_idxr_; }

    // get APIs for LIF related state
    slab *lif_pd_slab(void) const { return lif_pd_slab_; }
    indexer *lif_hwid_idxr(void) const { return lif_hwid_idxr_; }

    // get APIs for Uplinkif  related state
    slab *uplinkif_pd_slab(void) const { return uplinkif_pd_slab_; }
    indexer *uplinkifpc_hwid_idxr(void) const { return uplinkifpc_idxr_; }

    // get APIs for uplinkpc related state
    slab *uplinkpc_pd_slab(void) const { return uplinkpc_pd_slab_; }

    // get APIs for enicif  related state
    slab *enicif_pd_slab(void) const { return enicif_pd_slab_; }
    slab *if_l2seg_entry_slab(void) const { return if_l2seg_entry_slab_; }

    // get APIs for cpuif  related state
    slab *cpuif_pd_slab(void) const { return cpuif_pd_slab_; }

    // get APIs for TunnelIf related state
    slab *tunnelif_pd_slab(void) const { return tunnelif_pd_slab_; }

    // get APIs for LIF related state
    slab *ep_pd_slab(void) const { return ep_pd_slab_; }
    slab *ep_pd_ip_entry_slab(void) const { return ep_pd_ip_entry_slab_; }
    // indexer *rw_table_idxr(void) const { return rw_table_idxr_; }

    // get APIs for nwsec related state
    slab *nwsec_pd_slab(void) const { return nwsec_pd_slab_; }

    // get APIs for dos policy related state
    slab *dos_pd_slab(void) const { return dos_pd_slab_; }

    // get APIs for session related state
    slab *session_slab(void) const { return session_slab_; }

    // get APIs for TLS CB related state
    slab *tlscb_slab(void) const { return tlscb_slab_; }
    ht *tlscb_hwid_ht(void) const { return tlscb_hwid_ht_; }

    // get APIs for TCP CB related state
    slab *tcpcb_slab(void) const { return tcpcb_slab_; }
    ht *tcpcb_hwid_ht(void) const { return tcpcb_hwid_ht_; }

    // get APIs for Qos-class related state
    slab *qos_class_pd_slab(void) const { return qos_class_pd_slab_; }
    indexer *qos_iq_idxr(tm_port_type_e port_type) { return qos_iq_idxr_[port_type]; }
    indexer *qos_common_oq_idxr() { return qos_common_oq_idxr_; }
    indexer *qos_rxdma_oq_idxr() { return qos_rxdma_oq_idxr_; }

    // get APIs for Copp related state
    slab *copp_pd_slab(void) const { return copp_pd_slab_; }

    // get APIs for Acl related state
    slab *acl_pd_slab(void) const { return acl_pd_slab_; }

    // get APIs for WRING related state
    slab *wring_slab(void) const { return wring_slab_; }
    ht *wring_hwid_ht(void) const { return wring_hwid_ht_; }

    // get APIs for Crypto state
    indexer *crypto_pd_keys_idxr(void) { return session_keys_idxr_; }
    indexer *crypto_asym_dma_descr_idxr(void) { return crypto_asym_dma_descr_idxr_; }
    indexer *crypto_sym_msg_descr_idxr(void) { return crypto_sym_msg_descr_idxr_; }
    indexer *hbm_mem_idxr(void) { return hbm_mem_idxr_; }
    indexer *crypto_asym_key_descr_idxr(void) { return crypto_asym_key_descr_idxr_; }
    
    // get APIs for IPSEC CB related state
    slab *ipseccb_slab(void) const { return ipseccb_slab_; }
    ht *ipseccb_hwid_ht(void) const { return ipseccb_hwid_ht_; }

    slab *ipseccb_decrypt_slab(void) const { return ipseccb_decrypt_slab_; }
    ht *ipseccb_decrypt_hwid_ht(void) const { return ipseccb_decrypt_hwid_ht_; }

    // get APIs for L4LB related state
    slab *l4lb_pd_slab(void) const { return l4lb_pd_slab_; }
    
    // get APIs for TCP CB related state
    slab *cpucb_slab(void) const { return cpucb_slab_; }
    ht *cpucb_hwid_ht(void) const { return cpucb_hwid_ht_; }

    // get APIs for Raw Redirect CB related state
    slab *rawrcb_slab(void) const { return rawrcb_slab_; }
    ht *rawrcb_hwid_ht(void) const { return rawrcb_hwid_ht_; }

    // get APIs for Raw Chain CB related state
    slab *rawccb_slab(void) const { return rawccb_slab_; }
    ht *rawccb_hwid_ht(void) const { return rawccb_hwid_ht_; }

    // get APIs for Proxy Redirect CB related state
    slab *proxyrcb_slab(void) const { return proxyrcb_slab_; }
    ht *proxyrcb_hwid_ht(void) const { return proxyrcb_hwid_ht_; }

    // get APIs for Proxy Chain CB related state
    slab *proxyccb_slab(void) const { return proxyccb_slab_; }
    ht *proxyccb_hwid_ht(void) const { return proxyccb_hwid_ht_; }

    // get APIs for RW table related state
    slab *rw_entry_slab(void) const { return rw_entry_slab_; }
    ht *rw_table_ht(void) const { return rw_table_ht_; }
    indexer *rw_tbl_idxr(void) { return rw_tbl_idxr_; }

    // get APIs for Tunnel RW table related state
    slab *tnnl_rw_entry_slab(void) const { return tnnl_rw_entry_slab_; }
    ht *tnnl_rw_table_ht(void) const { return tnnl_rw_table_ht_; }
    indexer *tnnl_rw_tbl_idxr(void) { return tnnl_rw_tbl_idxr_; }

    // get APIs for CPU PKT related state
    slab *cpupkt_slab(void) const { return cpupkt_slab_; }
    slab *cpupkt_qinst_info_slab(void) const {return cpupkt_qinst_info_slab_; }
    indexer *cpupkt_descr_hw_id_idxr(void) {return cpupkt_descr_hwid_idxr_; }
    indexer *cpupkt_page_hw_id_idxr(void) {return cpupkt_page_hwid_idxr_; }

    // get APIs for TXS scheduler related state
    BMAllocator *txs_scheduler_map_idxr(void) { return txs_scheduler_map_idxr_; }    

    hal_ret_t init_tables(void);
    hal_ret_t p4plus_rxdma_init_tables(void);
    hal_ret_t p4plus_txdma_init_tables(void);
    directmap *dm_table(p4pd_table_id tid) const {
        if ((tid < P4TBL_ID_INDEX_MIN) || (tid > P4TBL_ID_INDEX_MAX)) {
            return NULL;
        }
        return dm_tables_[tid - P4TBL_ID_INDEX_MIN];
    }

    sdk_hash *hash_tcam_table(p4pd_table_id tid) const {
        if ((tid < P4TBL_ID_HASH_OTCAM_MIN) ||
            (tid > P4TBL_ID_HASH_OTCAM_MAX)) {
            return NULL;
        }
        return hash_tcam_tables_[tid - P4TBL_ID_HASH_OTCAM_MIN];
    }

    tcam *tcam_table(p4pd_table_id tid) const {
        if ((tid < P4TBL_ID_TCAM_MIN) || (tid > P4TBL_ID_TCAM_MAX)) {
            return NULL;
        }
        return tcam_tables_[tid - P4TBL_ID_TCAM_MIN];
    }

    Flow *flow_table(void) const {
        return flow_table_;
    }

    Met *met_table(void) const {
        return met_table_;
    }

    acl_tcam *acl_table(void) const {
        return acl_table_;
    }

    directmap *p4plus_rxdma_dm_table(p4pd_common_rxdma_actions_table_id tid) const {
        if ((tid < P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN) ||
            (tid > P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MAX)) {
            return NULL;
        }
        return p4plus_rxdma_dm_tables_[tid - P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN];
    }

    directmap *p4plus_txdma_dm_table(p4pd_common_txdma_actions_table_id tid) const {
        if ((tid < P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN) ||
            (tid > P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MAX)) {
            return NULL;
        }
        return p4plus_txdma_dm_tables_[tid - P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN];
    }

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
        slab       *vrf_slab_;
        indexer    *vrf_hwid_idxr_;
    } __PACK__;

    struct {
        indexer    *nwsec_profile_hwid_idxr_;
    } __PACK__;

    // l2seg related state
    struct {
        slab       *l2seg_slab_;
        ht         *flow_lkupid_ht_;
        indexer    *l2seg_cpu_idxr_;
    } __PACK__;

    // mc entry related state
    struct {
        slab       *mc_entry_slab_;
    } __PACK__;

    // Lport Indexer
    struct {
        indexer    *lport_idxr_;
    } __PACK__;

    // LIF related state
    struct {
        slab       *lif_pd_slab_;
        indexer    *lif_hwid_idxr_;         // Used even by Uplink IF/PCs
    } __PACK__;

    // Uplink IF related state
    struct {
        slab       *uplinkif_pd_slab_;
        slab       *uplinkpc_pd_slab_;
        indexer    *uplinkifpc_idxr_;       // User for both Uplink IF/PCs
    } __PACK__;

    // Enic IF related state
    struct {
        slab       *enicif_pd_slab_;
        slab       *if_l2seg_entry_slab_;
    } __PACK__;

    // CPU IF related state
    struct {
        slab       *cpuif_pd_slab_;
    } __PACK__;

    // Tunnel IF related state
    struct {
        slab       *tunnelif_pd_slab_;
    } __PACK__;

    // EP related state
    struct {
        slab       *ep_pd_slab_;
        slab       *ep_pd_ip_entry_slab_;
        // indexer    *rw_table_idxr_;
    } __PACK__;

    // nwsec related state
    struct {
        slab       *nwsec_pd_slab_;
    } __PACK__;

    // dos policy related state
    struct {
        slab       *dos_pd_slab_;
    } __PACK__;

    // session related state
    struct {
        slab       *session_slab_;
    } __PACK__;

    // tlscb related state
    struct {
        slab       *tlscb_slab_;
        ht         *tlscb_hwid_ht_;
    } __PACK__;

    // tcpcb related state
    struct {
        slab       *tcpcb_slab_;
        ht         *tcpcb_hwid_ht_;
    } __PACK__;

    // Qos related state
    struct {
        slab       *qos_class_pd_slab_;
        // Array of indexers for each of the tm port types - uplink, p4, dma
        indexer    **qos_iq_idxr_;
        indexer    *qos_common_oq_idxr_;
        indexer    *qos_rxdma_oq_idxr_;
        // Buffer island configuration
    } __PACK__;

    // Copp related state
    struct {
        slab       *copp_pd_slab_;
    } __PACK__;

    // Acl related state
    struct {
        slab       *acl_pd_slab_;
    } __PACK__;

    // wring related state
    struct {
        slab       *wring_slab_;
        ht         *wring_hwid_ht_;
    } __PACK__;

    // Crypto related state
    struct {
        indexer    *session_keys_idxr_;
        indexer    *crypto_asym_dma_descr_idxr_;
        indexer    *crypto_sym_msg_descr_idxr_;
        indexer    *hbm_mem_idxr_;
        indexer    *crypto_asym_key_descr_idxr_;
    } __PACK__;
    
    // ipseccb related state
    struct {
        slab       *ipseccb_slab_;
        ht         *ipseccb_hwid_ht_;
        slab       *ipseccb_decrypt_slab_;
        ht         *ipseccb_decrypt_hwid_ht_;
    } __PACK__;

    // l4lb related state
    struct {
        slab       *l4lb_pd_slab_;
    } __PACK__;

    // rw table management
    struct {
        slab      *rw_entry_slab_;
        ht        *rw_table_ht_;   
        indexer   *rw_tbl_idxr_;
    } __PACK__;

    // tnnl rw table management
    struct {
        slab      *tnnl_rw_entry_slab_;
        ht        *tnnl_rw_table_ht_;   
        indexer   *tnnl_rw_tbl_idxr_;
    } __PACK__;
    
    // cpucb related state
    struct {
        slab       *cpucb_slab_;
        ht         *cpucb_hwid_ht_;
    } __PACK__;

    // cpupkt related state
    struct {
        slab       *cpupkt_slab_;
        slab       *cpupkt_qinst_info_slab_;
        indexer    *cpupkt_descr_hwid_idxr_;
        indexer    *cpupkt_page_hwid_idxr_;
    } __PACK__;

    // Raw Redirect CB related state
    struct {
        slab       *rawrcb_slab_;
        ht         *rawrcb_hwid_ht_;
    } __PACK__;

    // Proxy Redirect CB related state
    struct {
        slab       *proxyrcb_slab_;
        ht         *proxyrcb_hwid_ht_;
    } __PACK__;

    // TXS scheduler related state
    struct {
        BMAllocator    *txs_scheduler_map_idxr_;
    } __PACK__;

    // Raw Chain CB related state
    struct {
        slab       *rawccb_slab_;
        ht         *rawccb_hwid_ht_;
    } __PACK__;

    // Proxy Chain CB related state
    struct {
        slab       *proxyccb_slab_;
        ht         *proxyccb_hwid_ht_;
    } __PACK__;

    directmap    **dm_tables_;
    sdk_hash     **hash_tcam_tables_;
    tcam         **tcam_tables_;
    Flow         *flow_table_;
    Met          *met_table_;
    acl_tcam     *acl_table_;
    directmap    **p4plus_rxdma_dm_tables_;
    directmap    **p4plus_txdma_dm_tables_;
};

hal_ret_t delay_delete_to_slab(hal_slab_t slab_id, void *elem);

} // namespace pd
} // namespace hal

#endif    // __HAL_STATE_PD_HPP__


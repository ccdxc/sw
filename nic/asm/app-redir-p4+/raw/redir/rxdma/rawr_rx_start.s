#include "app_redir_common.h"

struct phv_                                     p;
struct common_p4plus_stage0_app_header_table_k  k;
struct common_p4plus_stage0_app_header_table_d  d;

/*
 * Registers usage
 */
#define r_chain_rxq_base            r1
#define r_chain_txq_base            r2
#define r_pkt_len                   r3
#define r_alloc_inf_addr            r4
#define r_ring_indices_addr         r5
#define r_hash_key                  r6
#define r_pkt_type                  r7

/*
 * Register reuse (on early abort)
 */
#define r_qstate_addr               r7

%%
    .param          rawr_s1_desc_sem_pindex_post_update
    .param          rawr_s1_ppage_sem_pindex_post_update
    .param          rawr_s1_mpage_sem_pindex_post_update
    .param          rawr_err_stats_inc
    
    .align

rawr_s0_rx_start:
        
    /*
     * Stage 0 table engine always fetches the qstate (in d-vector)
     * for processing by the stage 0 MPUs.
     * 
     * In addition, 
     * R1 = table lookup hash value
     * R2 = packet size
     * R3 = random number
     * R4 = current time
     * R5 = programmable table constant
     * R6 = phv timestamp
     * R7 = qstate ring_not_empty
     */
     
    CAPRI_CLEAR_TABLE0_VALID

    /*
     * Two sentinels surround the programming of CB byte sequence:
     * rawccb_deactivate must be false and rawccb_activate must
     * be true to indicate readiness.
     */
    sne         c1, d.u.rawr_rx_start_d.rawrcb_deactivate, RAWRCB_DEACTIVATE
    seq.c1      c1, d.u.rawr_rx_start_d.rawrcb_activate, RAWRCB_ACTIVATE
    b.!c1       _rawrcb_not_ready
    phvwr       p.common_phv_rawrcb_flags,\
                d.{u.rawr_rx_start_d.rawrcb_flags}.hx // delay slot
    /*
     * For a given flow, one of 2 types of redirection applies:
     *   1) Redirect to ARM CPU RxQ, or
     *   2) Redirect to a P4+ TxQ
     */
    add         r_chain_rxq_base, r0, d.{u.rawr_rx_start_d.chain_rxq_base}.dx
    add         r_chain_txq_base, r0, d.{u.rawr_rx_start_d.chain_txq_base}.dx
    sne         c1, r_chain_rxq_base, r0
    sne         c2, r_chain_txq_base, r0
    bcf         [c1 & c2], _qstate_cfg_err_discard

    /*
     * Validate packet length
     *    Packet_len field contains
     *      sizeof(p4_to_p4plus_cpu_pkt_t) + complete packet length
     *    to which we will add an app header of size P4PLUS_RAW_REDIR_HDR_SZ
     */
    add         r_pkt_len, r0, k.{rawr_app_header_packet_len_sbit0_ebit5...\
                                  rawr_app_header_packet_len_sbit6_ebit13} // delay slot
    ble.s       r_pkt_len, r0, _packet_len_err_discard
    addi        r_pkt_len, r_pkt_len, P4PLUS_RAW_REDIR_HDR_SZ // delay slot
    phvwrpair   p.common_phv_packet_len, r_pkt_len, \
                p.common_phv_qstate_addr, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR
    /*
     * qid is our flow ID context
     */
    bcf         [!c1 & !c2], _qstate_cfg_err_discard
    phvwr       p.pen_raw_redir_hdr_v1_flow_id, CAPRI_RXDMA_INTRINSIC_QID // delay slot
    bcf         [c1], _chain_rxq_set
    phvwrpair   p.common_phv_chain_ring_index_select,\
                d.u.rawr_rx_start_d.chain_txq_ring_index_select[2:0],\
                p.common_phv_chain_ring_base, r_chain_txq_base  // delay slot
    phvwrpair   p.common_phv_chain_ring_size_shift, d.u.rawr_rx_start_d.chain_txq_ring_size_shift[4:0],\
                p.common_phv_chain_entry_size_shift, d.u.rawr_rx_start_d.chain_txq_entry_size_shift[4:0]
    phvwr       p.t1_s2s_chain_lif, d.{u.rawr_rx_start_d.chain_txq_lif}.hx
    phvwr       p.t1_s2s_chain_qtype, d.u.rawr_rx_start_d.chain_txq_qtype
    phvwr       p.t1_s2s_chain_qid, d.{u.rawr_rx_start_d.chain_txq_qid}.wx
    b           _r_ring_indices_addr_check
    add         r_ring_indices_addr, r0, d.{u.rawr_rx_start_d.chain_txq_ring_indices_addr}.dx // delay slot

_chain_rxq_set:
    phvwrpair   p.common_phv_chain_to_rxq, TRUE,\
                p.common_phv_chain_ring_base, r_chain_rxq_base
    phvwrpair   p.common_phv_chain_ring_size_shift, d.u.rawr_rx_start_d.chain_rxq_ring_size_shift[4:0],\
                p.common_phv_chain_entry_size_shift, d.u.rawr_rx_start_d.chain_rxq_entry_size_shift[4:0]
    phvwr       p.common_phv_chain_ring_index_select, d.u.rawr_rx_start_d.chain_rxq_ring_index_select
    add         r_ring_indices_addr, r0, d.{u.rawr_rx_start_d.chain_rxq_ring_indices_addr}.dx

_r_ring_indices_addr_check:
    beq         r_ring_indices_addr, r0, _qstate_cfg_err_discard
    phvwr       p.t1_s2s_chain_ring_indices_addr, r_ring_indices_addr // delay slot

    /*
     * qid 0 is dedicated for SPAN and, in which case,
     * perform symmetric hash to determine which ...
     */
    seq         c3, CAPRI_RXDMA_INTRINSIC_QID, APP_REDIR_SPAN_RAWRCB_ID // delay slot
    bcf         [!c3], _page_alloc_prep
    phvwr.c3    p.common_phv_redir_span_instance, TRUE // delay slot
    
    /*
     * Set up HW toeplitz hash for SPAN;
     * currently, L3/L4 app header data is available only when the SPAN
     * method uses ingress multicast replication (i.e., not mirror session)
     */
#if !APP_REDIR_VISIBILITY_USE_MIRROR_SESSION
     
    TOEPLITZ_KEY_DATA_SETUP(CPU_HASH_KEY_PREFIX,
                            P4PLUS_APPTYPE_RAW_REDIR,
                            rawr_app_header,
                            r_pkt_type,
                            r_hash_key,
                            _page_alloc_prep)
_page_alloc_prep:

#endif
    
    /*
     * Allocate either one meta page, or one packet page, or both.
     */
    blei        r_pkt_len, APP_REDIR_MPAGE_SIZE, _mpage_sem_pindex_fetch_update
    nop
    
_ppage_sem_pindex_fetch_update:

    /*
     * Fetch/update memory page pindex for storing packet
     */
    addi        r_alloc_inf_addr, r0, CAPRI_SEM_RNMPR_ALLOC_INF_ADDR
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS,
                          rawr_s1_ppage_sem_pindex_post_update,
                          r_alloc_inf_addr,
                          TABLE_SIZE_64_BITS)
    blei        r_pkt_len, APP_REDIR_PPAGE_SIZE, _desc_sem_pindex_fetch_update
    nop
    
/*
 * Fetch/update memory page pindex for storing metaheader
 */
_mpage_sem_pindex_fetch_update:

    addi        r_alloc_inf_addr, r0, CAPRI_SEM_RNMPR_SMALL_ALLOC_INF_ADDR
    CAPRI_NEXT_TABLE_READ(2, TABLE_LOCK_DIS,
                          rawr_s1_mpage_sem_pindex_post_update,
                          r_alloc_inf_addr,
                          TABLE_SIZE_64_BITS)

/*
 * Fetch/update buffer descriptor pindex for queuing to chain ring
 */
_desc_sem_pindex_fetch_update:

    addi        r_alloc_inf_addr, r0, CAPRI_SEM_RNMDR_ALLOC_INF_ADDR
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                          rawr_s1_desc_sem_pindex_post_update,
                          r_alloc_inf_addr,
                          TABLE_SIZE_64_BITS)
    nop.e
    nop

/*
 * Discard packet due to invalid packet length;
 */
_packet_len_err_discard:

    RAWRCB_ERR_STAT_INC_LAUNCH(3, r_qstate_addr,
                               CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR,
                               p.t3_s2s_inc_stat_pkt_len_err)
    phvwr.e     p.p4_intr_global_drop, 1
    nop    

    
/*
 * Discard packet due to error in qstate configuration
 */
_qstate_cfg_err_discard:

    RAWRCB_ERR_STAT_INC_LAUNCH(3, r_qstate_addr,
                               CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR,
                               p.t3_s2s_inc_stat_qstate_cfg_err)
    phvwr.e     p.p4_intr_global_drop, 1
    nop


/*
 * CB not currently activated
 */
_rawrcb_not_ready:     

    RAWRCB_ERR_STAT_INC_LAUNCH(3, r_qstate_addr,
                               CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR,
                               p.t3_s2s_inc_stat_cb_not_ready)
    phvwr.e     p.p4_intr_global_drop, 1
    nop


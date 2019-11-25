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
 * Register reuse
 */
#define r_qstate_addr               r7

%%
    .param          rawr_ppage_sem_pindex_post_update
    .param          rawr_cb_extra_read
     RAWR_METRICS_PARAMS()
    
    .align

rawr_rx_start:
        
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
     

    /*
     * Two sentinels surround the programming of CB byte sequence:
     * rawccb_deactivate must be false and rawccb_activate must
     * be true to indicate readiness.
     */
    sne         c1, d.u.rawr_rx_start_d.rawrcb_deactivate, RAWRCB_DEACTIVATE
    seq.c1      c1, d.u.rawr_rx_start_d.rawrcb_activate, RAWRCB_ACTIVATE
    b.!c1       _rawrcb_not_ready
    phvwr       p.rawr_kivec0_rawrcb_flags,\
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
    bgti        r_pkt_len, RAWR_RNMDPR_USABLE_PAGE_SIZE, _packet_len_err_discard
    phvwrpair   p.rawr_kivec0_packet_len, r_pkt_len, \
                p.rawr_kivec0_qstate_addr, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR // delay slot
    /*
     * qid is our flow ID context
     */
    bcf         [!c1 & !c2], _qstate_cfg_err_discard
    phvwr       p.pen_raw_redir_hdr_v1_flow_id, CAPRI_RXDMA_INTRINSIC_QID // delay slot
    bcf         [c1], _chain_rxq_set
    phvwrpair   p.rawr_kivec0_chain_ring_index_select,\
                d.u.rawr_rx_start_d.chain_txq_ring_index_select[2:0],\
                p.rawr_kivec0_chain_ring_size_shift, d.u.rawr_rx_start_d.chain_txq_ring_size_shift[4:0]
    phvwrpair   p.rawr_kivec2_chain_ring_base, r_chain_txq_base, \
                p.rawr_kivec2_chain_entry_size_shift, d.u.rawr_rx_start_d.chain_txq_entry_size_shift[4:0]
    phvwr       p.rawr_kivec2_chain_lif, d.{u.rawr_rx_start_d.chain_txq_lif}.hx
    phvwr       p.rawr_kivec2_chain_qtype, d.u.rawr_rx_start_d.chain_txq_qtype
    phvwr       p.rawr_kivec2_chain_qid, d.{u.rawr_rx_start_d.chain_txq_qid}.wx
    b           _r_ring_indices_addr_check
    add         r_ring_indices_addr, r0, d.{u.rawr_rx_start_d.chain_txq_ring_indices_addr}.dx // delay slot

_chain_rxq_set:
    phvwrpair   p.rawr_kivec0_chain_ring_size_shift, d.u.rawr_rx_start_d.chain_rxq_ring_size_shift[4:0], \
                p.rawr_kivec0_chain_to_rxq, TRUE
    phvwrpair   p.rawr_kivec2_chain_ring_base, r_chain_rxq_base, \
                p.rawr_kivec2_chain_entry_size_shift, d.u.rawr_rx_start_d.chain_rxq_entry_size_shift[4:0]
    phvwr       p.rawr_kivec0_chain_ring_index_select, d.u.rawr_rx_start_d.chain_rxq_ring_index_select
    add         r_ring_indices_addr, r0, d.{u.rawr_rx_start_d.chain_rxq_ring_indices_addr}.dx

_r_ring_indices_addr_check:
    beq         r_ring_indices_addr, r0, _qstate_cfg_err_discard
    phvwr       p.rawr_kivec1_chain_ring_indices_addr, r_ring_indices_addr // delay slot

    /*
     * qid 0 is dedicated for SPAN and, in which case,
     * perform symmetric hash to determine which ...
     */
    seq         c3, CAPRI_RXDMA_INTRINSIC_QID, APP_REDIR_SPAN_RAWRCB_ID // delay slot
    bcf         [!c3], _page_alloc_prep
    phvwr.c3    p.rawr_kivec0_redir_span_instance, TRUE // delay slot
    
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
#endif
    
_page_alloc_prep:

    /*
     * Fetch/update memory page pindex for storing packet
     */
    addi        r_alloc_inf_addr, r0, RAWR_RNMDPR_ALLOC_IDX  // delay slot
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                          rawr_ppage_sem_pindex_post_update,
                          r_alloc_inf_addr,
                          TABLE_SIZE_64_BITS)
    /*
     * Launch read of cb_extra portion
     */                          
    add         r_qstate_addr, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR, \
                RAWRCB_TABLE_EXTRA_OFFSET
    CAPRI_NEXT_TABLE_READ_e(1,
                            TABLE_LOCK_DIS,
                            rawr_cb_extra_read,
                            r_qstate_addr,
                            TABLE_SIZE_512_BITS)
    nop

/*
 * Discard packet due to invalid packet length;
 */
_packet_len_err_discard:

    RAWR_METRICS_SET(pkt_len_discards)
    b           _error_metrics_launch
    CAPRI_CLEAR_TABLE0_VALID                    // delay slot

/*
 * Discard packet due to error in qstate configuration
 */
_qstate_cfg_err_discard:

    RAWR_METRICS_SET(qstate_cfg_discards)
    b           _error_metrics_launch
    CAPRI_CLEAR_TABLE0_VALID                    // delay slot


/*
 * CB not currently activated
 */
_rawrcb_not_ready:     

    RAWR_METRICS_SET(cb_not_ready_discards)
    b           _error_metrics_launch
    CAPRI_CLEAR_TABLE0_VALID                    // delay slot

_error_metrics_launch:

    /*
     * On any of the above errors, there would have been no tables launched.
     * Hence, we must launch of metrics tables update/commit here. (Subsequent
     * stages launched for the same tables would be required to relaunch the
     * the affected metrics tables update.)
     *
     * NOTE: due to the use of HW toeplitz hash which reserves stage 3 to
     * handle the callback from rx_table_cpu_hash, any normal launches of
     * metrics tables update/commit must start from or after stage 3.
     */
    RAWR_METRICS0_TABLE2_COMMIT_e(CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR)


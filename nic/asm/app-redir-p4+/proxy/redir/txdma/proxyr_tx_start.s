#include "app_redir_common.h"

struct phv_                     p;
struct proxyr_tx_start_k        k;
struct proxyr_tx_start_d        d;

/*
 * Registers usage
 */
#define r_ci                        r1  // my_txq onsumer index
#define r_pi                        r2  // my_txq producer index
#define r_my_txq_qid                r3
#define r_qstate_addr               r4
#define r_return                    r5
#define r_db_addr_scratch           r6
#define r_db_data_scratch           r7

/*
 * Registers reuse (post meta headers prefill)
 */
#define r_my_txq_slot               r_my_txq_qid
 
%%
    .param          proxyr_s1_my_txq_entry_consume
    .param          proxyr_s1_flow_key_post_read
    .param          proxyr_err_stats_inc
    
    .align

proxyr_s0_tx_start:
        
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
    seq         c1, r7[APP_REDIR_PROXYR_RINGS_MAX-1:0], r0
    bcf         [c1], _my_txq_ring_empty

    /*
     * qid is our flow ID context
     */
    phvwrpair   p.to_s1_my_txq_lif, CAPRI_INTRINSIC_LIF, \
                p.to_s1_my_txq_qtype, CAPRI_TXDMA_INTRINSIC_QTYPE // delay slot
    phvwrpair   p.to_s1_my_txq_ring_size_shift, d.u.start_d.my_txq_ring_size_shift, \
                p.to_s1_my_txq_qid, CAPRI_TXDMA_INTRINSIC_QID
                
    phvwrpair   p.common_phv_chain_ring_index_select, d.u.start_d.chain_rxq_ring_index_select[2:0], \
                p.common_phv_qstate_addr, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR
    phvwr       p.common_phv_redir_span_instance, d.u.start_d.redir_span
    phvwr       p.common_phv_proxyrcb_flags, d.{u.start_d.proxyrcb_flags}.hx
    
    /*
     * Also prefill certain meta header fields;
     * FTE expects p4plus_cpu_pkt_lif/qtype/qid in host order,
     * whereas L7 header fields should all be in network order
     */
    phvwr       p.p4plus_cpu_pkt_lif, CAPRI_INTRINSIC_LIF.hx
    phvwr       p.p4plus_cpu_pkt_qtype, CAPRI_TXDMA_INTRINSIC_QTYPE
    add         r_my_txq_qid, r0, CAPRI_TXDMA_INTRINSIC_QID
    phvwr       p.p4plus_cpu_pkt_qid, r_my_txq_qid.wx
    phvwr       p.pen_proxyr_hdr_v1_flow_id, r_my_txq_qid

    phvwr       p.common_phv_chain_ring_base, d.{u.start_d.chain_rxq_base}.dx
    phvwrpair   p.common_phv_chain_ring_size_shift, d.u.start_d.chain_rxq_ring_size_shift[4:0], \
                p.common_phv_chain_entry_size_shift, d.u.start_d.chain_rxq_entry_size_shift[4:0]
    phvwr       p.to_s4_chain_ring_indices_addr, d.{u.start_d.chain_rxq_ring_indices_addr}.dx
    
    /*
     * Two sentinels surround the programming of CB byte sequence:
     * proxyrcb_deactivate must be false and proxyrcb_activate must
     * be true to indicate readiness.
     *
     * Note that proxyrcb_activate is part of proxyrcb_extra_t and
     * its evaluation will be deferred until proxyrcb_extra_t is read
     * in stage 1.
     */
    phvwr       p.to_s1_proxyrcb_deactivate, d.u.start_d.proxyrcb_deactivate
    add         r_qstate_addr, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, \
                PROXYRCB_TABLE_FLOW_KEY_OFFSET                  
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS,
                          proxyr_s1_flow_key_post_read,
                          r_qstate_addr,
                          TABLE_SIZE_512_BITS)
    /*
     * PI assumed to have been incremented by doorbell write by a producer program
     * and verified above with r7 when program is entered.
     *
     * Launch read of descriptor at current CI.
     * Note that my_txq_base and corresponding CI/PI, once programmed,
     * are never cleared (because doing so can cause Tx scheduler lockup).
     * What can get cleared is the proxyrcb_active flag which would tell this
     * program to properly consume and free the descriptor along with 
     * any pages embedded within.
     */
    add         r_ci, r0, d.{u.start_d.ci_0}.hx
    sllv        r_my_txq_slot, r_ci, d.u.start_d.my_txq_entry_size_shift
    add         r_my_txq_slot, r_my_txq_slot, d.{u.start_d.my_txq_base}.dx
    CAPRI_NEXT_TABLE_READ(0,
                          TABLE_LOCK_DIS,
                          proxyr_s1_my_txq_entry_consume,
                          r_my_txq_slot,
                          TABLE_SIZE_64_BITS)
                          
                          
    tblmincri.f d.{u.start_d.ci_0}.hx, d.u.start_d.my_txq_ring_size_shift, 1
    mincr       r_ci, d.u.start_d.my_txq_ring_size_shift, 1
    add         r_pi, r0, d.{u.start_d.pi_0}.hx
    mincr       r_pi, d.u.start_d.my_txq_ring_size_shift, r0

    /*
     * if new CI now == PI, clear scheduler bit
     */
    bne         r_ci, r_pi, _proxyrcb_done
    DOORBELL_NO_UPDATE(CAPRI_INTRINSIC_LIF, 
                       CAPRI_TXDMA_INTRINSIC_QTYPE, 
                       CAPRI_TXDMA_INTRINSIC_QID,
                       r_db_addr_scratch, r_db_data_scratch)
_proxyrcb_done:
    nop.e
    nop    
    
     
/*
 * Early exit: my TxQ ring actually empty when entered
 */
_my_txq_ring_empty:

#ifdef APP_REDIR_COLLECT_MY_TXQ_EMPTY_STATS
    PROXYRCB_ERR_STAT_INC_LAUNCH(3, r_qstate_addr,
                                 CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR,
                                 p.t3_s2s_inc_stat_txq_empty)
#endif
    phvwr.e     p.p4_intr_global_drop, 1
    nop

    

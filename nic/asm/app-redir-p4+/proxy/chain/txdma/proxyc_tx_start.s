#include "app_redir_common.h"

struct phv_                     p;
struct proxyc_tx_start_k        k;
struct proxyc_tx_start_start_d  d;

/*
 * Registers usage
 */
#define r_ci                        r1  // my_txq onsumer index
#define r_pi                        r2  // my_txq producer index
#define r_my_txq_slot               r3
#define r_return                    r4
#define r_qstate_addr               r5
#define r_db_addr_scratch           r6
#define r_db_data_scratch           r7

%%
    .param          proxyc_s1_my_txq_entry_consume
    .param          proxyc_err_stats_inc
    
    .align

proxyc_s0_tx_start:
        
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
    phvwr       p.common_phv_qstate_addr, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR
    seq         c1, r7[APP_REDIR_PROXYC_RINGS_MAX-1:0], r0
    bcf         [c1], _my_txq_ring_empty
    
    /*
     * qid is our flow ID context
     */
    phvwr       p.to_s1_my_txq_lif, CAPRI_INTRINSIC_LIF     // delay slot
    phvwr       p.to_s1_my_txq_qtype, CAPRI_TXDMA_INTRINSIC_QTYPE
    phvwr       p.to_s1_my_txq_qid, CAPRI_TXDMA_INTRINSIC_QID

    phvwr       p.common_phv_chain_txq_base, d.{chain_txq_base}.dx
    phvwr       p.common_phv_chain_txq_ring_size_shift, d.chain_txq_ring_size_shift
    phvwr       p.common_phv_chain_txq_entry_size_shift, d.chain_txq_entry_size_shift
    phvwr       p.t0_s2s_chain_txq_lif, d.{chain_txq_lif}.hx
    phvwr       p.t0_s2s_chain_txq_qtype, d.chain_txq_qtype
    phvwr       p.t0_s2s_chain_txq_qid, d.{chain_txq_qid}.wx
    phvwr       p.t0_s2s_chain_txq_ring, d.chain_txq_ring
    
    phvwr       p.to_s2_chain_txq_ring_indices_addr, d.{chain_txq_ring_indices_addr}.dx
    phvwr       p.to_s1_my_txq_ring_size_shift, d.my_txq_ring_size_shift
    
    /*
     * Two sentinels surround the programming of CB byte sequence:
     * proxyccb_deactivate must be false and proxyccb_activate must
     * be true to indicate readiness.
     */
    sne         c1, d.proxyccb_deactivate, PROXYCCB_DEACTIVATE
    seq.c1      c1, d.proxyccb_activate, PROXYCCB_ACTIVATE
    bal.!c1     r_return, _proxyccb_not_ready
    phvwr       p.common_phv_proxyccb_flags, d.{proxyccb_flags}.hx // delay slot
    
    /*
     * PI assumed to have been incremented by doorbell write by a producer program
     * and verified above with r7 when program is entered.
     *
     * Launch read of descriptor at current CI.
     * Note that my_txq_base and corresponding CI/PI, once programmed,
     * are never cleared (because doing so can cause Tx scheduler lockup).
     * What can get cleared is the proxyccb_active flag which would tell this
     * program to properly consume and free the descriptor along with 
     * any pages embedded within.
     */
    add         r_ci, r0, d.{ci_0}.hx
    sllv        r_my_txq_slot, r_ci, d.my_txq_entry_size_shift
    add         r_my_txq_slot, r_my_txq_slot, d.{my_txq_base}.dx
    CAPRI_NEXT_TABLE_READ(0,
                          TABLE_LOCK_DIS,
                          proxyc_s1_my_txq_entry_consume,
                          r_my_txq_slot,
                          TABLE_SIZE_64_BITS)
    
    tblmincri.f d.{ci_0}.hx, d.my_txq_ring_size_shift, 1
    mincr       r_ci, d.my_txq_ring_size_shift, 1
    add         r_pi, r0, d.{pi_0}.hx
    mincr       r_pi, d.my_txq_ring_size_shift, r0
    
    /*
     * if new CI now == PI, clear scheduler bit
     */
    bne         r_ci, r_pi, _proxyccb_done
    DOORBELL_NO_UPDATE(CAPRI_INTRINSIC_LIF, 
                       CAPRI_TXDMA_INTRINSIC_QTYPE, 
                       CAPRI_TXDMA_INTRINSIC_QID,
                       r_db_addr_scratch, r_db_data_scratch)
_proxyccb_done:
    nop.e
    nop    


/*
 * CB has been de-activated or never made ready
 */
_proxyccb_not_ready:
 
    PROXYCCB_ERR_STAT_INC_LAUNCH(3, r_qstate_addr,
                                 CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR,
                                 p.t3_s2s_inc_stat_cb_not_ready)
    jr          r_return
    phvwri      p.common_phv_do_cleanup_discard, TRUE   // delay slot
     
    
/*
 * Early exit: my TxQ ring actually empty when entered
 */
_my_txq_ring_empty:

    PROXYCCB_ERR_STAT_INC_LAUNCH(3, r_qstate_addr,
                                 CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR,
                                 p.t3_s2s_inc_stat_my_txq_empty)
    phvwr.e     p.p4_intr_global_drop, 1
    nop    

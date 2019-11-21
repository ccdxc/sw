#include "app_redir_common.h"

struct phv_                     p;
struct s0_tbl_k                 k;
struct s0_tbl_rawc_tx_start_d   d;

/*
 * Registers usage
 */
#define r_qstate_addr               r2
#define r_ci                        r3  // my_txq onsumer index
#define r_pi                        r4  // my_txq producer index
#define r_my_txq_slot               r5
#define r_db_addr_scratch           r6
#define r_db_data_scratch           r7

%%
    .param          rawc_my_txq_entry_consume
    .param          rawc_cb_extra_read
     RAWC_METRICS_PARAMS()
    
    .align

rawc_tx_start:
        
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
     
    seq         c1, r7[APP_REDIR_RAWC_RINGS_MAX-1:0], r0
    bcf         [c1], _my_txq_ring_empty
    
    /*
     * Two sentinels surround the programming of CB byte sequence:
     * rawccb_deactivate must be false and rawccb_activate must
     * be true to indicate readiness.
     */
    sne         c1, d.rawccb_deactivate, RAWCCB_DEACTIVATE      // delay slot
    seq.c1      c1, d.rawccb_activate, RAWCCB_ACTIVATE
    bcf         [!c1], _rawccb_not_ready
    phvwr       p.rawc_kivec0_qstate_addr, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR    // delay slot
    
    phvwr       p.rawc_kivec1_chain_txq_ring_indices_addr, d.{chain_txq_ring_indices_addr}.dx
                
    phvwr       p.rawc_kivec0_rawccb_flags, d.{rawccb_flags}.hx
    phvwrpair   p.rawc_kivec0_chain_txq_ring_size_shift, d.chain_txq_ring_size_shift, \
                p.rawc_kivec0_chain_txq_entry_size_shift, d.chain_txq_entry_size_shift
                
    phvwr       p.rawc_kivec2_chain_txq_base, d.{chain_txq_base}.dx
    phvwr       p.rawc_kivec2_chain_txq_lif, d.{chain_txq_lif}.hx
    phvwr       p.rawc_kivec2_chain_txq_qid, d.{chain_txq_qid}.wx
    phvwrpair   p.rawc_kivec2_chain_txq_qtype, d.chain_txq_qtype[2:0], \
                p.rawc_kivec2_chain_txq_ring, d.chain_txq_ring[2:0]
    
    sne         c2, d.chain_txq_base, r0
    phvwri.c2   p.rawc_kivec0_next_service_chain_action, TRUE
    
    /*
     * PI assumed to have been incremented by doorbell write by a producer program
     * and verified above with r7 when program is entered.
     *
     * Launch read of descriptor at current CI.
     * Note that my_txq_base and corresponding CI/PI, once programmed,
     * are never cleared (because doing so can cause Tx scheduler lockup).
     * What can get altered are the rawccb_deactivated and rawccb_activated
     * flags which would tell this program to properly consume and free
     * the descriptor along with any pages embedded within.
     */    
    add         r_ci, r0, d.{ci_0}.hx
    sllv        r_my_txq_slot, r_ci, d.my_txq_entry_size_shift
    add         r_my_txq_slot, r_my_txq_slot, d.{my_txq_base}.dx
    CAPRI_NEXT_TABLE_READ(0,
                          TABLE_LOCK_DIS,
                          rawc_my_txq_entry_consume,
                          r_my_txq_slot,
                          TABLE_SIZE_64_BITS)
    /*
     * Launch read of cb_extra portion
     */                          
    add         r_qstate_addr, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, \
                RAWCCB_TABLE_EXTRA_OFFSET
    CAPRI_NEXT_TABLE_READ(1,
                          TABLE_LOCK_DIS,
                          rawc_cb_extra_read,
                          r_qstate_addr,
                          TABLE_SIZE_512_BITS)
    mincr       r_ci, d.my_txq_ring_size_shift, 1
    add         r_pi, r0, d.{pi_0}.hx
    mincr       r_pi, d.my_txq_ring_size_shift, r0

    /*
     * if new CI now == PI, clear scheduler bit
     */
    sne         c1, r_ci, r_pi
    nop.c1.e
    tblmincri.f d.{ci_0}.hx, d.my_txq_ring_size_shift, 1        // delay slot

_db_eval_update:    
    DOORBELL_NO_UPDATE(CAPRI_INTRINSIC_LIF, 
                       CAPRI_TXDMA_INTRINSIC_QTYPE, 
                       CAPRI_TXDMA_INTRINSIC_QID,
                       r_db_addr_scratch, r_db_data_scratch)
_metrics_launch:

    /*
     * Initiate launch of metrics tables update/commit here. Subsequent stages
     * launched for the same tables would be required to relaunch the
     * the affected metrics tables update.
     */
    RAWC_METRICS0_TABLE2_COMMIT_e(CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR)
    
/*
 * CB has been de-activated or never made ready
 */
_rawccb_not_ready:
 
    DOORBELL_NO_UPDATE_DISABLE_SCHEDULER(CAPRI_INTRINSIC_LIF, 
                                         CAPRI_TXDMA_INTRINSIC_QTYPE, 
                                         CAPRI_TXDMA_INTRINSIC_QID,
                                         r0, r_db_addr_scratch, r_db_data_scratch)
    RAWC_METRICS_SET(cb_not_ready_discards)
    b           _metrics_launch
    CAPRI_CLEAR_TABLE0_VALID                    // delay slot
     
 
/*
 * Early exit: my TxQ ring actually empty when entered
 */
_my_txq_ring_empty:

#ifdef APP_REDIR_COLLECT_MY_TXQ_EMPTY_STATS
    RAWC_METRICS_SET(my_txq_empty_discards)
#endif
    b           _db_eval_update
    CAPRI_CLEAR_TABLE0_VALID                    // delay slot

#include "session_age_common.h"

struct phv_                             p;
struct s0_tbl_k                         k;
struct s0_tbl_age_start_d               d;

/*
 * Table engine always fetches the qstate (in d-vector)
 * for processing by the stage MPU.
 * 
 * In addition, 
 * R1 = table lookup hash value
 * R2 = packet size
 * R3 = random number
 * R4 = current time
 * R5 = programmable table constant
 * R6 = phv timestamp
 * R7 = qstate ring[7:0] not_empty
 */
 
/*
 * Registers usage
 */
#define r_qstate_addr                   r3
#define r_stage                         r4
#define r_qid                           r5
#define r_db_addr                       r6
#define r_db_data                       r7

%%
    .param          age_fsm_exec
    .param          age_summarize
     AGE_METRICS_PARAMS()
    
    .align

session_age_start:

    seq         c1, r7[0], r0
    bcf         [c1], _age_ring_empty
    add         r_qid, CAPRI_TXDMA_INTRINSIC_QID, r0            // delay slot
    
    phvwr       p.age_kivec0_qstate_addr, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR
    phvwrpair   p.age_kivec7_lif, CAPRI_INTRINSIC_LIF, \
                p.age_kivec7_qtype, CAPRI_TXDMA_INTRINSIC_QTYPE
    AGE_DB_DATA(r_qid) 
    phvwr       p.db_data_no_index_data, r_db_data.dx
    
    sne         c1, d.cb_activate, SESSION_AGE_CB_ACTIVATE
    bcf         [c1], _age_cb_cfg_err
    phvwr       p.poller_slot_data_scanner_qid, r_qid.wx        // delay slot
    
    AGE_KIVEC_MAX_AGE_MS_LOAD(2)            
    
    AGE_DB_DATA_TIMER(r_qid, d.scan_resched_time)
    phvwr       p.db_data_batch_timer_data, r_db_data.dx
    
    AGE_DB_DATA_TIMER(r_qid, SESSION_AGE_POLLER_QFULL_REPOST_TIME)
    phvwr       p.db_data_repost_timer_data, r_db_data.dx
    
    /*
     * Launch state machine
     */                          
    add         r_qstate_addr, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, \
                SESSION_AGE_CB_TABLE_FSM_OFFSET
    LOAD_TABLE_FOR_ADDR(0,
                        TABLE_LOCK_EN,
                        r_qstate_addr,
                        TABLE_SIZE_512_BITS,
                        age_fsm_exec)

_metrics_launch:
                          
    /*
     * Consume ring slot
     */                          
    tblwr.f     d.ci_0, d.pi_0
    
    /*
     * Initiate launch of metrics table update/commit here. Subsequent stages
     * launched for the same table(s) would be required to relaunch the
     * the affected metrics table(s).
     */
    AGE_METRICS0_TABLE3_COMMIT_LAUNCH_e(CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR)
    
    .align
    
/*
 * CB not ready or has config errors
 */
_age_cb_cfg_err:
 
    AGE_METRICS_SET(cb_cfg_err_discards)
    AGE_SUMMARIZE_LAUNCH(0, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR,
                         age_summarize)
    b           _metrics_launch
    phvwr       p.age_kivec0_cb_cfg_err_discard, 1      // delay slot
     
 
/*
 * Early exit: ring empty when entered
 */
_age_ring_empty:

    phvwr.e     p.p4_intr_global_drop, 1
    CLEAR_TABLE0                                        // delay slot
    

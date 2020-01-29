#include "ftl_dev_shared_asm.h"

struct phv_                             p;
struct s0_tbl_k                         k;
struct s0_tbl_session_start_d           d;

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
    .param          session_fsm_exec
    .param          session_norm_tmo_load
    .param          session_accel_tmo_load
    .param          session_summarize
     SESSION_METRICS_PARAMS()
    
    .align

scanner_session_start:

    seq         c1, r7[0], r0
    bcf         [c1], _scanner_ring_empty
    add         r_qid, HW_MPU_TXDMA_INTRINSIC_QID, r0            // delay slot
    
    phvwr       p.session_kivec0_qstate_addr, HW_MPU_TXDMA_INTRINSIC_QSTATE_ADDR
    phvwrpair   p.session_kivec7_lif, HW_MPU_INTRINSIC_LIF, \
                p.session_kivec7_qtype, HW_MPU_TXDMA_INTRINSIC_QTYPE
    SCANNER_DB_DATA(r_qid) 
    phvwr       p.db_data_no_index_data, r_db_data.dx
    
    sne         c1, d.cb_activate, SCANNER_SESSION_CB_ACTIVATE
    bcf         [c1], _scanner_cb_cfg_discard
    phvwr       p.poller_slot_data_scanner_qid, r_qid.wx        // delay slot
    
    SCANNER_DB_DATA_TIMER(r_qid, d.scan_resched_time)
    phvwr       p.db_data_burst_timer_data, r_db_data.dx
    
    SCANNER_DB_DATA_TIMER(r_qid, SCANNER_SESSION_POLLER_QFULL_REPOST_TIME)
    phvwr       p.db_data_qfull_repost_timer_data, r_db_data.dx
    
    /*
     * Launch state machine
     */                          
    add         r_qstate_addr, HW_MPU_TXDMA_INTRINSIC_QSTATE_ADDR, \
                SCANNER_SESSION_CB_TABLE_FSM_OFFSET
    LOAD_TABLE_FOR_ADDR(0,
                        TABLE_LOCK_EN,
                        r_qstate_addr,
                        TABLE_SIZE_512_BITS,
                        session_fsm_exec)
    LOAD_TABLE_FOR_ADDR64(1,
                          TABLE_LOCK_DIS,
                          d.normal_tmo_cb_addr,
                          TABLE_SIZE_512_BITS,
                          session_norm_tmo_load)
    LOAD_TABLE_FOR_ADDR64(2,
                          TABLE_LOCK_DIS,
                          d.accel_tmo_cb_addr,
                          TABLE_SIZE_512_BITS,
                          session_accel_tmo_load)
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
    SESSION_METRICS0_TABLE3_COMMIT_LAUNCH_e(HW_MPU_TXDMA_INTRINSIC_QSTATE_ADDR)
    
/*
 * Discard due to control block configuration
 */
_scanner_cb_cfg_discard:
 
    SESSION_METRICS_SET(cb_cfg_discards)
    SESSION_SUMMARIZE_LAUNCH(0, HW_MPU_TXDMA_INTRINSIC_QSTATE_ADDR,
                             session_summarize)
    b           _metrics_launch
    phvwr       p.session_kivec0_cb_cfg_discard, 1          // delay slot
     
 
/*
 * Early exit: ring empty when entered
 */
_scanner_ring_empty:

    phvwr.e     p.p4_intr_global_drop, 1
    CLEAR_TABLE0                                        // delay slot
    

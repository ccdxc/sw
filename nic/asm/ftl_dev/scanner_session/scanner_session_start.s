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

    SCANNER_DB_ADDR_SCHED_EVAL(HW_MPU_INTRINSIC_LIF,
                               HW_MPU_TXDMA_INTRINSIC_QTYPE)
.brbegin
    brpri       r7[SCANNER_RING_MAX-1:0], [1, 0]
    add         r_qid, HW_MPU_TXDMA_INTRINSIC_QID, r0            // delay slot
    
.brcase SCANNER_RING_NORMAL
    
    /*
     * Consume ring slot and issue scheduler update eval
     */                          
    tblwr.f     d.ci_0, d.pi_0
    SCANNER_DB_DATA_WITH_RING(r_qid, SCANNER_RING_NORMAL) 

_session_start:
    
    memwr.dx    r_db_addr, r_db_data
    phvwr       p.db_data_no_index_data, r_db_data.dx

    phvwrpair   p.session_kivec0_qstate_addr, HW_MPU_TXDMA_INTRINSIC_QSTATE_ADDR, \
                p.session_kivec0_qtype, HW_MPU_TXDMA_INTRINSIC_QTYPE[0]
    phvwr       p.session_kivec7_lif, HW_MPU_INTRINSIC_LIF
    
    SCANNER_DB_DATA_TIMER_WITH_RING(HW_MPU_TXDMA_INTRINSIC_QTYPE,
                                    r_qid, SCANNER_RING_TIMER,
                                    SCANNER_POLLER_QFULL_REPOST_TICKS)
    phvwr       p.db_data_qfull_repost_ticks_data, r_db_data.dx
    
    SCANNER_DB_DATA_TIMER_WITH_RING(HW_MPU_TXDMA_INTRINSIC_QTYPE,
                                    r_qid, SCANNER_RING_TIMER,
                                    SCANNER_RANGE_EMPTY_RESCHED_TICKS)
    phvwr       p.db_data_range_empty_ticks_data, r_db_data.dx
    
    phvwr       p.poller_slot_data_scanner_qid, r_qid.wx
    sne         c1, d.cb_activate, SCANNER_SESSION_CB_ACTIVATE
    bcf         [c1], _scanner_cb_cfg_discard
    phvwr       p.poller_slot_data_scanner_qtype, HW_MPU_TXDMA_INTRINSIC_QTYPE // delay slot
    
    SCANNER_DB_DATA_TIMER_WITH_RING(HW_MPU_TXDMA_INTRINSIC_QTYPE,
                                    r_qid, SCANNER_RING_TIMER,
                                    d.scan_resched_ticks)
    phvwr       p.db_data_burst_ticks_data, r_db_data.dx
    phvwr       p.session_kivec8_resched_uses_slow_timer, \
                d.resched_uses_slow_timer
    
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
                          
    DMA_CMD_PTR_INIT(dma_p2m_0)
    
    /*
     * Initiate launch of metrics table update/commit here. Subsequent stages
     * launched for the same table(s) would be required to relaunch the
     * the affected metrics table(s).
     */
    SESSION_METRICS_SET(scan_invocations)
    SESSION_METRICS0_TABLE3_COMMIT_LAUNCH_e(HW_MPU_TXDMA_INTRINSIC_QSTATE_ADDR)
    
_scanner_cb_cfg_discard:
 
    /*
     * Discard due to control block configuration
     */
    SESSION_METRICS_SET(cb_cfg_discards)
    SESSION_SUMMARIZE_LAUNCH(0, HW_MPU_TXDMA_INTRINSIC_QSTATE_ADDR,
                             session_summarize)
    b           _metrics_launch
    phvwr       p.session_kivec0_cb_cfg_discard, 1      // delay slot
     
.brcase SCANNER_RING_TIMER
    
    /*
     * Consume ring slot and issue schedulder update eval (on branch)
     */                          
    SCANNER_DB_DATA_WITH_RING(r_qid, SCANNER_RING_TIMER) 
    b           _session_start
    tblwr.f     d.ci_1, d.pi_1                          // delay slot
    
.brcase SCANNER_RING_MAX
    
    /*
     * Early exit: rings empty when entered
     */
    phvwr.e     p.p4_intr_global_drop, 1
    CLEAR_TABLE0                                        // delay slot
    
.brend
    

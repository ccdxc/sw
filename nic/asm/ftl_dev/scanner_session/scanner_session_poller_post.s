#include "ftl_dev_shared_asm.h"

struct phv_                             p;
struct s7_tbl_k                         k;
struct s7_tbl_session_poller_post_d     d;

/*
 * Registers usage
 */
#define r_ci                            r1
#define r_pi_shadow                     r2
#define r_qstate_addr                   r3
#define r_poller_slot_addr              r4
#define r_fsm_state_next                r5
#define r_db_addr                       r6
#define r_scratch                       r7

    
%%
    .align

/*
 * Entered when
 * 1) range_full=1 and current expiry_maps are zero
 */   
session_poller_empty_post_eval:

    // If nothing had ever been posted before for the range, don't post anything
    // and simply reschedule with a timer delay for the next range rescan.
    // Otherwise, post an empty map anyway with request for poller to
    // reschedule us.

    bbne        SESSION_KIVEC8_RANGE_HAS_POSTED, 0, session_poller_post
    CLEAR_TABLE0                                                // delay slot

#ifndef HW

    // On SIM platform (debug capability enabled), even though
    // step_tmr_wheel_update() appears to function correctly, it is
    // rather funky (due to the model's handling of ctime) so let poller 
    // reschedule us rather than using a timer.
    or          r_scratch, SESSION_KIVEC0_FORCE_SESSION_EXPIRED_TS, \
                           SESSION_KIVEC0_FORCE_CONNTRACK_EXPIRED_TS
    bne         r_scratch, r0, session_poller_post
#endif    
    phvwri      p.fsm_state_next_state, SCANNER_STATE_RESTART_RANGE // delay slot
    
    // DMA0 = update FSM state
    
    add         r_qstate_addr, SESSION_KIVEC0_QSTATE_ADDR, \
                SCANNER_SESSION_CB_TABLE_FSM_OFFSET + \
                SCANNER_STRUCT_BYTE_OFFS(s1_tbl_session_fsm_exec_d, fsm_state)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_0_dma_cmd,
                                r_qstate_addr,
                                fsm_state_next_state,
                                fsm_state_next_state)

    // When using timer, send an update eval to quiet the scheduler
    // (unless scheduling was already disabled prior).
    //
    // DMA1 = scheduler update eval
    
    SCANNER_DB_ADDR_SCHED_EVAL(SESSION_KIVEC7_LIF,
                               SESSION_KIVEC0_QTYPE)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_1_dma_cmd,
                                r_db_addr,
                                db_data_no_index_data,
                                db_data_no_index_data)

    // DMA2 = Reschedule with a range empty delay
    
    SCANNER_DB_ADDR_TIMER(SCANNER_RANGE_EMPTY_RESCHED_TIMER,
                          SESSION_KIVEC7_LIF)
    CAPRI_DMA_CMD_PHV2MEM_SETUP_STOP_FENCE_e(dma_p2m_2_dma_cmd,
                                             r_db_addr,
                                             db_data_range_empty_ticks_data,
                                             db_data_range_empty_ticks_data)
                                       
    .align
    
/*
 * Entered when
 * 1) range_full=1 (and some non-zero expiry_maps had been posted prior), or
 * 2) expiry_maps_full=1 and current expiry_maps are non-zero
 */   
session_poller_post:

    CLEAR_TABLE0
    add         r_pi_shadow, d.pi_0_shadow, r0
    add         r_ci, d.ci_0, r0
    mincr       r_pi_shadow, d.qdepth_shft, 1
    mincr       r_ci, d.qdepth_shft, r0
    beq         r_pi_shadow, r_ci, _poller_qfull
    
    // If completed one full sweep (i.e., range_full), 
    //
    // DMA0 = scheduler update eval
    // i.e., idle scheduling and tell poller to reschedule us. The goal here
    // is to prevent duplicate postings to the software poller.
    
_if4:    
    sne         c3, SESSION_KIVEC8_RANGE_FULL, r0               // delay slot
    bcf         [!c3], _endif4
    phvwr.c3    p.poller_slot_data_flags, SCANNER_RESCHED_REQUESTED // delay slot
    SCANNER_DB_ADDR_SCHED_EVAL(SESSION_KIVEC7_LIF,
                               SESSION_KIVEC0_QTYPE)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_0_dma_cmd,
                                r_db_addr,
                                db_data_no_index_data,
                                db_data_no_index_data)
_endif4:    
                                      
    // Calculate slot address at which to write post data 
    
    tbladd      d.num_qposts, 1
    tblmincri.f d.pi_0_shadow, d.qdepth_shft, 1
    phvwr       p.poller_posted_pi_pi, r_pi_shadow.hx
    mincr       r_pi_shadow, d.qdepth_shft, -1
    add         r_poller_slot_addr, d.wring_base_addr, r_pi_shadow, \
                POLLER_SLOT_DATA_BYTES_SHFT

    // DMA1 = poller slot data (i.e., expiry_maps, etc.)
    
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_1_dma_cmd,
                                r_poller_slot_addr,
                                poller_slot_data_table_id_base,
                                poller_slot_data_flags)
                                
    // DMA2 = clear FSM expiry_maps
    
    add         r_qstate_addr, SESSION_KIVEC0_QSTATE_ADDR, \
                SCANNER_SESSION_CB_TABLE_SUMMARIZE_OFFSET + \
                SCANNER_STRUCT_BYTE_OFFS(s6_tbl_session_summarize_d, expiry_map0)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_2_dma_cmd,
                                r_qstate_addr,
                                poller_null_data_expiry_map0,
                                poller_null_data_expiry_map3)
                                
    // DMA3 = update FSM state
    
    add         r_qstate_addr, SESSION_KIVEC0_QSTATE_ADDR, \
                SCANNER_SESSION_CB_TABLE_FSM_OFFSET + \
                SCANNER_STRUCT_BYTE_OFFS(s1_tbl_session_fsm_exec_d, fsm_state)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_3_dma_cmd,
                                r_qstate_addr,
                                fsm_state_next_state,
                                fsm_state_next_state)

    // DMA4 = set or clear indication of whether we have posted any expiry maps.
    // Since we're posting, set indication to 1 unless we're at range end,
    // in which case, clear it.
     
    phvwr.!c3    p.poller_range_has_posted_posted, 1
    add         r_qstate_addr, SESSION_KIVEC0_QSTATE_ADDR, \
                SCANNER_SESSION_CB_TABLE_SUMMARIZE_OFFSET + \
                SCANNER_STRUCT_BYTE_OFFS(s6_tbl_session_summarize_d, range_has_posted)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_4_dma_cmd,
                                r_qstate_addr,
                                poller_range_has_posted_posted,
                                poller_range_has_posted_posted)
    
    // Reschedule if not range_full;
    
_if6:    
    bcf         [c3], _endif6
    cmov        r_fsm_state_next, c3, SCANNER_STATE_RESTART_RANGE, \
                                      SCANNER_STATE_RESTART_EXPIRY_MAP // delay slot
_if8:
    bbeq        SESSION_KIVEC8_BURST_FULL, 1, _else8
                                    
    // DMA5 = reschedule with a PI increment
    
    SCANNER_DB_ADDR_SCHED_PIDX_INC(SESSION_KIVEC7_LIF,
                                   SESSION_KIVEC0_QTYPE)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_5_dma_cmd,
                                r_db_addr,
                                db_data_no_index_data,
                                db_data_no_index_data)
    b           _endif8
    nop
                                        
_else8:
                                             
    // Alternatively, reschedule with a burst timer delay.
    // When using timer, send an update eval to quiet the scheduler
    //
    // DMA5 = scheduler update eval
    
    SCANNER_DB_ADDR_SCHED_EVAL(SESSION_KIVEC7_LIF,
                               SESSION_KIVEC0_QTYPE)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_5_dma_cmd,
                                r_db_addr,
                                db_data_no_index_data,
                                db_data_no_index_data)
                                
    // DMA6 = burst timer delay
    
    SCANNER_DB_ADDR_FAST_OR_SLOW_TIMER(SESSION_KIVEC8_RESCHED_USES_SLOW_TIMER,
                                       SESSION_KIVEC7_LIF)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_6_dma_cmd,
                                r_db_addr,
                                db_data_burst_ticks_data,
                                db_data_burst_ticks_data)
_endif8:
    CAPRI_DMA_CMD_FENCE(dma_p2m_5_dma_cmd)
    
_endif6:
    
    // DMA7 = increment poller PI last to ensure that, if poller were to
    // reschedule us, all our state would have been updated prior.
                                
    add         r_qstate_addr, SESSION_KIVEC7_POLLER_QSTATE_ADDR, \
                SCANNER_STRUCT_BYTE_OFFS(s7_tbl_session_poller_post_d, pi_0)
    CAPRI_DMA_CMD_PHV2MEM_SETUP_STOP(dma_p2m_7_dma_cmd,
                                     r_qstate_addr,
                                     poller_posted_pi_pi,
                                     poller_posted_pi_pi)
    phvwr.e     p.fsm_state_next_state, r_fsm_state_next
    CAPRI_DMA_CMD_FENCE_COND(dma_p2m_7_dma_cmd, c3)     // delay slot
    
/*
 * Software poller queue is full, next fsm_state is to reevaluate
 */
_poller_qfull:

    tbladd      d.num_qfulls, 1
    phvwr       p.fsm_state_next_state, SCANNER_STATE_REEVALUATE

    // DMA0 = update FSM state
    
    add         r_qstate_addr, SESSION_KIVEC0_QSTATE_ADDR, \
                SCANNER_SESSION_CB_TABLE_FSM_OFFSET + \
                SCANNER_STRUCT_BYTE_OFFS(s1_tbl_session_fsm_exec_d, fsm_state)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_0_dma_cmd,
                                r_qstate_addr,
                                fsm_state_next_state,
                                fsm_state_next_state)
                                
    // When using timer, send an update eval to quiet the scheduler
    // (unless scheduling was already disabled prior).
    //
    // DMA1 = scheduler update eval
    
    SCANNER_DB_ADDR_SCHED_EVAL(SESSION_KIVEC7_LIF,
                               SESSION_KIVEC0_QTYPE)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_1_dma_cmd,
                                r_db_addr,
                                db_data_no_index_data,
                                db_data_no_index_data)
    // DMA2 = Reschedule with a repost delay
    
    SCANNER_DB_ADDR_TIMER(SCANNER_POLLER_QFULL_REPOST_TIMER,
                          SESSION_KIVEC7_LIF)
    CAPRI_DMA_CMD_PHV2MEM_SETUP_STOP_FENCE_e(dma_p2m_2_dma_cmd,
                                             r_db_addr,
                                             db_data_qfull_repost_ticks_data,
                                             db_data_qfull_repost_ticks_data)


    .align
    
/*
 * Entered when there's nothing to post to software poller, because
 * 1) It's not yet time to post (i.e., range_full=0 and expiry_maps_full=0), or
 * 2) expiry_maps are all zero (and range_full=0)
 */
session_fsm_state_eval:

    CLEAR_TABLE0
    seq         c1, SESSION_KIVEC8_EXPIRY_MAPS_FULL, r0
    cmov        r_fsm_state_next, c1, SCANNER_STATE_REEVALUATE, \
                                      SCANNER_STATE_RESTART_EXPIRY_MAP 
    
    // Leave the current expiry_maps untouched and only update state
    // DMA0 = update FSM state
    
    add         r_qstate_addr, SESSION_KIVEC0_QSTATE_ADDR, \
                SCANNER_SESSION_CB_TABLE_FSM_OFFSET + \
                SCANNER_STRUCT_BYTE_OFFS(s1_tbl_session_fsm_exec_d, fsm_state)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_0_dma_cmd,
                                r_qstate_addr,
                                fsm_state_next_state,
                                fsm_state_next_state)
                                
    bbeq        SESSION_KIVEC8_BURST_FULL, 1, _burst_full_resched
    phvwr       p.fsm_state_next_state, r_fsm_state_next        // delay slot
                                    
    // DMA1 = reschedule with a PI increment
    
    SCANNER_DB_ADDR_SCHED_PIDX_INC(SESSION_KIVEC7_LIF,
                                   SESSION_KIVEC0_QTYPE)
    CAPRI_DMA_CMD_PHV2MEM_SETUP_STOP_FENCE_e(dma_p2m_1_dma_cmd,
                                             r_db_addr,
                                             db_data_no_index_data,
                                             db_data_no_index_data)
_burst_full_resched:
                                             
    // Alternatively, eschedule with a burst timer delay.
    // When using timer, send an update eval to quiet the scheduler
    // (unless scheduling was already disabled prior).
    //
    // DMA1 = scheduler update eval
    
    SCANNER_DB_ADDR_SCHED_EVAL(SESSION_KIVEC7_LIF,
                               SESSION_KIVEC0_QTYPE)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_1_dma_cmd,
                                r_db_addr,
                                db_data_no_index_data,
                                db_data_no_index_data)
    // DMA2 = reschedule with a burst timer delay
    
    SCANNER_DB_ADDR_FAST_OR_SLOW_TIMER(SESSION_KIVEC8_RESCHED_USES_SLOW_TIMER,
                                       SESSION_KIVEC7_LIF)
    CAPRI_DMA_CMD_PHV2MEM_SETUP_STOP_FENCE_e(dma_p2m_2_dma_cmd,
                                             r_db_addr,
                                             db_data_burst_ticks_data,
                                             db_data_burst_ticks_data)

    .align
    
/*
 * Control block not ready or error - revert to initial state and 
 * idle scheduler.
 */
session_scan_idle:
 
    CLEAR_TABLE0
                
    // DMA0 = clear FSM expiry_maps

    add         r_qstate_addr, SESSION_KIVEC0_QSTATE_ADDR, \
                SCANNER_SESSION_CB_TABLE_SUMMARIZE_OFFSET + \
                SCANNER_STRUCT_BYTE_OFFS(s6_tbl_session_summarize_d, expiry_map0)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_0_dma_cmd,
                                r_qstate_addr,
                                poller_null_data_expiry_map0,
                                poller_null_data_expiry_map3)
                                
    // DMA1 = update FSM state
    
    phvwr       p.fsm_state_next_state, SCANNER_STATE_INITIAL
    add         r_qstate_addr, SESSION_KIVEC0_QSTATE_ADDR, \
                SCANNER_SESSION_CB_TABLE_FSM_OFFSET + \
                SCANNER_STRUCT_BYTE_OFFS(s1_tbl_session_fsm_exec_d, fsm_state)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_1_dma_cmd,
                                r_qstate_addr,
                                fsm_state_next_state,
                                fsm_state_next_state)
                                
    // DMA2 = clear indication of whether we have posted any expiry maps
    
    phvwr       p.poller_range_has_posted_posted, 0
    add         r_qstate_addr, SESSION_KIVEC0_QSTATE_ADDR, \
                SCANNER_SESSION_CB_TABLE_SUMMARIZE_OFFSET + \
                SCANNER_STRUCT_BYTE_OFFS(s6_tbl_session_summarize_d, range_has_posted)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_2_dma_cmd,
                                r_qstate_addr,
                                poller_range_has_posted_posted,
                                poller_range_has_posted_posted)
                                
    // DMA3 = scheduler update eval
    
    SCANNER_DB_ADDR_SCHED_EVAL(SESSION_KIVEC7_LIF,
                               SESSION_KIVEC0_QTYPE)
    CAPRI_DMA_CMD_PHV2MEM_SETUP_STOP_FENCE_e(dma_p2m_3_dma_cmd,
                                             r_db_addr,
                                             db_data_no_index_data,
                                             db_data_no_index_data)
                                             


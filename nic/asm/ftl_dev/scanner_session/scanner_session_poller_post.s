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

    
%%
    .align

/*
 * Entered when
 * 1) range_full=1, or
 * 2) expiry_maps_full=1 and current expiry_maps are non-zero
 */   
session_poller_post:

    CLEAR_TABLE0
    add         r_pi_shadow, d.pi_0_shadow, r0
    add         r_ci, d.ci_0, r0
    mincr       r_pi_shadow, d.qdepth_shft, 1
    mincr       r_ci, d.qdepth_shft, r0
    beq         r_pi_shadow, r_ci, _poller_qfull
    DMA_CMD_PTR_INIT(dma_p2m_0)                                 // delay slot
    
    // If completed one full sweep (i.e., range_full), 
    //
    // DMA0 = disable scheduler
    // i.e., stop scheduling and tell poller to reschedule us. The goal here
    // is to prevent duplicate postings to the software poller.
    
_if4:    
    sne         c3, SESSION_KIVEC8_SESSION_RANGE_FULL, r0
    bcf         [!c3], _endif4
    phvwr.c3    p.poller_slot_data_flags, SCANNER_RESCHED_REQUESTED     // delay slot
    SCANNER_DB_ADDR_SCHED_RESET(SESSION_KIVEC7_LIF,
                                SESSION_KIVEC7_QTYPE)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_0_dma_cmd,
                                r_db_addr,
                                db_data_no_index_data,
                                db_data_no_index_data)
_endif4:    
                                      
    // Calculate slot address at which to write post data 
    
    tblmincri.f d.pi_0_shadow, d.qdepth_shft, 1
    phvwr       p.poller_posted_pi_pi, r_pi_shadow
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

    // Reschedule if not range_full;
    
_if6:    
    bcf         [c3], _endif6
    cmov        r_fsm_state_next, c3, SCANNER_STATE_RESTART_RANGE, \
                                      SCANNER_STATE_RESTART_EXPIRY_MAP // delay slot

_if8:
    bbeq        SESSION_KIVEC8_SESSION_BURST_FULL, 1, _else8
                                    
    // DMA4 = reschedule with a PI increment
    
    SCANNER_DB_ADDR_SCHED_PIDX_INC(SESSION_KIVEC7_LIF,
                                   SESSION_KIVEC7_QTYPE)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_4_dma_cmd,
                                r_db_addr,
                                db_data_no_index_data,
                                db_data_no_index_data)
    b           _endif8
    nop
                                        
_else8:
                                             
    // Alternatively, DMA4 = reschedule with a burst timer delay.
    // (Scheduler already disabled above so no need to sched eval.)
    
    SCANNER_DB_ADDR_FAST_TIMER(SESSION_KIVEC7_LIF)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_4_dma_cmd,
                                r_db_addr,
                                db_data_burst_timer_data,
                                db_data_burst_timer_data)
_endif8:
    CAPRI_DMA_CMD_FENCE(dma_p2m_4_dma_cmd)
    
_endif6:
    
    // DMA5 = increment poller PI last to ensure that, if poller were to
    // reschedule us, all our state would have been updated prior.
                                
    add         r_qstate_addr, SESSION_KIVEC7_POLLER_QSTATE_ADDR, \
                SCANNER_STRUCT_BYTE_OFFS(s7_tbl_session_poller_post_d, pi_0)
    CAPRI_DMA_CMD_PHV2MEM_SETUP_STOP(dma_p2m_5_dma_cmd,
                                     r_qstate_addr,
                                     poller_posted_pi_pi,
                                     poller_posted_pi_pi)
    phvwr.e     p.fsm_state_next_state, r_fsm_state_next
    CAPRI_DMA_CMD_FENCE_COND(dma_p2m_5_dma_cmd, c3)     // delay slot
    
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
                               SESSION_KIVEC7_QTYPE)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_1_dma_cmd,
                                r_db_addr,
                                db_data_no_index_data,
                                db_data_no_index_data)
    // DMA2 = Reschedule with a repost delay
    
    SCANNER_DB_ADDR_FAST_TIMER(SESSION_KIVEC7_LIF)
    CAPRI_DMA_CMD_PHV2MEM_SETUP_STOP_FENCE_e(dma_p2m_2_dma_cmd,
                                             r_db_addr,
                                             db_data_qfull_repost_timer_data,
                                             db_data_qfull_repost_timer_data)


    .align
    
/*
 * Entered when there's nothing to post to software poller, because
 * 1) It's not yet time to post (i.e., range_full=0 and expiry_maps_full=0), or
 * 2) expiry_maps are all zero (and range_full=0)
 */
session_fsm_state_eval:

    CLEAR_TABLE0
    DMA_CMD_PTR_INIT(dma_p2m_0)
                
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
                                
    bbeq        SESSION_KIVEC8_SESSION_BURST_FULL, 1, _burst_full_resched
    phvwr       p.fsm_state_next_state, r_fsm_state_next        // delay slot
                                    
    // DMA1 = reschedule with a PI increment
    
    SCANNER_DB_ADDR_SCHED_PIDX_INC(SESSION_KIVEC7_LIF,
                                   SESSION_KIVEC7_QTYPE)
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
                               SESSION_KIVEC7_QTYPE)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_1_dma_cmd,
                                r_db_addr,
                                db_data_no_index_data,
                                db_data_no_index_data)
    // DMA2 = reschedule with a burst timer delay
    
    SCANNER_DB_ADDR_FAST_TIMER(SESSION_KIVEC7_LIF)
    CAPRI_DMA_CMD_PHV2MEM_SETUP_STOP_FENCE_e(dma_p2m_2_dma_cmd,
                                             r_db_addr,
                                             db_data_burst_timer_data,
                                             db_data_burst_timer_data)

    .align
    
/*
 * Control block not ready or error - disable queue scheduling
 */
session_scan_disable:
 
    CLEAR_TABLE0
    DMA_CMD_PTR_INIT(dma_p2m_0)
                
    // DMA0 = update FSM state
    
    phvwr       p.fsm_state_next_state, SCANNER_STATE_INITIAL
    add         r_qstate_addr, SESSION_KIVEC0_QSTATE_ADDR, \
                SCANNER_SESSION_CB_TABLE_FSM_OFFSET + \
                SCANNER_STRUCT_BYTE_OFFS(s1_tbl_session_fsm_exec_d, fsm_state)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_0_dma_cmd,
                                r_qstate_addr,
                                fsm_state_next_state,
                                fsm_state_next_state)
                                    
    // DMA1 = disable scheduler
    
    SCANNER_DB_ADDR_SCHED_RESET(SESSION_KIVEC7_LIF,
                                SESSION_KIVEC7_QTYPE)
    CAPRI_DMA_CMD_PHV2MEM_SETUP_STOP_FENCE_e(dma_p2m_1_dma_cmd,
                                             r_db_addr,
                                             db_data_no_index_data,
                                             db_data_no_index_data)


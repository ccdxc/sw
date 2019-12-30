#include "session_age_common.h"

struct phv_                             p;
struct s7_tbl_k                         k;
struct s7_tbl_age_poller_post_d         d;

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
age_poller_post:

    CLEAR_TABLE0
    add         r_pi_shadow, d.pi_0_shadow, r0
    add         r_ci, d.ci_0, r0
    mincr       r_pi_shadow, d.qdepth_shft, 1
    mincr       r_ci, d.qdepth_shft, r0
    beq         r_pi_shadow, r_ci, _poller_qfull
    phvwri      p.p4_txdma_intr_dma_cmd_ptr, \
                CAPRI_PHV_START_OFFSET(dma_p2m_0_dma_cmd_type) / 16 // delay slot
    
    // If completed one full sweep (i.e., range_full), 
    //
    // DMA0 = disable scheduler
    // i.e., stop scheduling and tell poller to reschedule us. The goal here
    // is to prevent duplicate postings to the software poller.
    
_if4:    
    sne         c3, AGE_KIVEC8_SESSION_RANGE_FULL, r0
    bcf         [!c3], _endif4
    phvwr.c3    p.poller_slot_data_flags, \
                SESSION_AGE_SCANNER_RESCHED_REQUESTED           // delay slot
    AGE_DB_ADDR_SCHED_RESET(AGE_KIVEC7_LIF,
                            AGE_KIVEC7_QTYPE)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_0_dma_cmd,
                                r_db_addr,
                                db_data_no_index_data,
                                db_data_no_index_data)
_endif4:    
                                      
    // Calculate slot address at which to write post data 
    
    tblmincri.f d.pi_0_shadow, d.qdepth_shft, 1
    phvwr       p.poller_posted_pi_pi, r_pi_shadow
    mincr       r_pi_shadow, d.qdepth_shft, -1
    add         r_poller_slot_addr, d.qbase_addr, r_pi_shadow, \
                SESSION_AGE_POLLER_SLOT_BYTES_SHFT

    // DMA1 = poller slot data (i.e., expiry_maps, etc.)
    
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_1_dma_cmd,
                                r_poller_slot_addr,
                                poller_slot_data_session_id_base,
                                poller_slot_data_flags)
                                
    // DMA2 = clear FSM expiry_maps
    
    add         r_qstate_addr, AGE_KIVEC0_QSTATE_ADDR, \
                SESSION_AGE_CB_TABLE_SUMMARIZE_OFFSET + \
                SIZE_IN_BYTES(AGE_BIT_OFFS_STRUCT(s6_tbl_age_summarize_d, expiry_map0))
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_2_dma_cmd,
                                r_qstate_addr,
                                poller_null_data_expiry_map0,
                                poller_null_data_expiry_map3)
                                
    // DMA3 = update FSM state
    
    add         r_qstate_addr, AGE_KIVEC0_QSTATE_ADDR, \
                SESSION_AGE_CB_TABLE_FSM_OFFSET + \
                SIZE_IN_BYTES(AGE_BIT_OFFS_STRUCT(s1_tbl_age_fsm_exec_d, fsm_state))
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_3_dma_cmd,
                                r_qstate_addr,
                                fsm_state_next_state,
                                fsm_state_next_state)

    // Reschedule if not range_full;
    
_if6:    
    bcf         [c3], _endif6
    cmov        r_fsm_state_next, c3, SESSION_AGE_STATE_RESTART_RANGE, \
                                      SESSION_AGE_STATE_RESTART_EXPIRY_MAP // delay slot

_if8:
    bbeq        AGE_KIVEC8_SESSION_BATCH_FULL, 1, _else8
                                    
    // DMA4 = reschedule with a PI increment
    
    AGE_DB_ADDR_SCHED_PIDX_INC(AGE_KIVEC7_LIF,
                               AGE_KIVEC7_QTYPE)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_4_dma_cmd,
                                r_db_addr,
                                db_data_no_index_data,
                                db_data_no_index_data)
    b           _endif8
    nop
                                        
_else8:
                                             
    // Alternatively, DMA4 = reschedule with a batch timer delay
    
    AGE_DB_ADDR_FAST_TIMER(AGE_KIVEC7_LIF)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_4_dma_cmd,
                                r_db_addr,
                                db_data_batch_timer_data,
                                db_data_batch_timer_data)
_endif8:
    CAPRI_DMA_CMD_FENCE(dma_p2m_4_dma_cmd)
    
_endif6:
    
    // DMA5 = increment poller PI last to ensure that, if poller were to
    // reschedule us, all our state would have been updated prior.
                                
    add         r_qstate_addr, AGE_KIVEC7_POLLER_QSTATE_ADDR, \
                SIZE_IN_BYTES(AGE_BIT_OFFS_STRUCT(s7_tbl_age_poller_post_d, pi_0))
    CAPRI_DMA_CMD_PHV2MEM_SETUP_STOP(dma_p2m_5_dma_cmd,
                                     r_qstate_addr,
                                     poller_posted_pi_pi,
                                     poller_posted_pi_pi)

    // Also use fence_fence if we're expecting poller to reschedule us,
    // to ensure all DMA state updates have been flushed.

    phvwr.e     p.fsm_state_next_state, r_fsm_state_next
    phvwr.c3    p.dma_p2m_5_dma_cmd_fence_fence, 1      // delay slot
    
    
/*
 * Software poller queue is full, next fsm_state is to reevaluate
 */
_poller_qfull:

    tbladd      d.num_qfulls, 1
    phvwr       p.fsm_state_next_state, SESSION_AGE_STATE_REEVALUATE

    // DMA0 = update FSM state
    
    add         r_qstate_addr, AGE_KIVEC0_QSTATE_ADDR, \
                SESSION_AGE_CB_TABLE_FSM_OFFSET + \
                SIZE_IN_BYTES(AGE_BIT_OFFS_STRUCT(s1_tbl_age_fsm_exec_d, fsm_state))
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_0_dma_cmd,
                                r_qstate_addr,
                                fsm_state_next_state,
                                fsm_state_next_state)
                                
    // DMA1 = Reschedule with a repost delay
    
    AGE_DB_ADDR_FAST_TIMER(AGE_KIVEC7_LIF)
    CAPRI_DMA_CMD_PHV2MEM_SETUP_STOP_FENCE_e(dma_p2m_1_dma_cmd,
                                             r_db_addr,
                                             db_data_repost_timer_data,
                                             db_data_repost_timer_data)


    .align
    
/*
 * Entered when there's nothing to post to software poller, because
 * 1) It's not yet time to post (i.e., range_full=0 and expiry_maps_full=0), or
 * 2) expiry_maps are all zero (and range_full=0)
 */
age_fsm_state_eval:

    CLEAR_TABLE0
    phvwri      p.p4_txdma_intr_dma_cmd_ptr, \
                CAPRI_PHV_START_OFFSET(dma_p2m_0_dma_cmd_type) / 16
                
    seq         c1, AGE_KIVEC8_EXPIRY_MAPS_FULL, r0
    cmov        r_fsm_state_next, c1, SESSION_AGE_STATE_REEVALUATE, \
                                      SESSION_AGE_STATE_RESTART_EXPIRY_MAP 
    
    // Leave the current expiry_maps untouched and only update state
    // DMA0 = update FSM state
    
    add         r_qstate_addr, AGE_KIVEC0_QSTATE_ADDR, \
                SESSION_AGE_CB_TABLE_FSM_OFFSET + \
                SIZE_IN_BYTES(AGE_BIT_OFFS_STRUCT(s1_tbl_age_fsm_exec_d, fsm_state))
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_0_dma_cmd,
                                r_qstate_addr,
                                fsm_state_next_state,
                                fsm_state_next_state)
                                
    bbeq        AGE_KIVEC8_SESSION_BATCH_FULL, 1, _batch_full_resched
    phvwr       p.fsm_state_next_state, r_fsm_state_next        // delay slot
                                    
    // DMA1 = reschedule with a PI increment
    
    AGE_DB_ADDR_SCHED_PIDX_INC(AGE_KIVEC7_LIF,
                               AGE_KIVEC7_QTYPE)
    CAPRI_DMA_CMD_PHV2MEM_SETUP_STOP_FENCE_e(dma_p2m_1_dma_cmd,
                                             r_db_addr,
                                             db_data_no_index_data,
                                             db_data_no_index_data)
_batch_full_resched:
                                             
    // Alternatively, DMA1 = reschedule with a batch timer delay
    
    AGE_DB_ADDR_FAST_TIMER(AGE_KIVEC7_LIF)
    CAPRI_DMA_CMD_PHV2MEM_SETUP_STOP_FENCE_e(dma_p2m_1_dma_cmd,
                                             r_db_addr,
                                             db_data_batch_timer_data,
                                             db_data_batch_timer_data)

    .align
    
/*
 * Control block not ready or error - disable queue scheduling
 */
age_scan_disable:
 
    CLEAR_TABLE0
    phvwri      p.p4_txdma_intr_dma_cmd_ptr, \
                CAPRI_PHV_START_OFFSET(dma_p2m_0_dma_cmd_type) / 16
                
    // DMA0 = update FSM state
    
    phvwr       p.fsm_state_next_state, SESSION_AGE_STATE_RESTART_RANGE
    add         r_qstate_addr, AGE_KIVEC0_QSTATE_ADDR, \
                SESSION_AGE_CB_TABLE_FSM_OFFSET + \
                SIZE_IN_BYTES(AGE_BIT_OFFS_STRUCT(s1_tbl_age_fsm_exec_d, fsm_state))
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_p2m_0_dma_cmd,
                                r_qstate_addr,
                                fsm_state_next_state,
                                fsm_state_next_state)
                                    
    // DMA1 = disable scheduler
    
    AGE_DB_ADDR_SCHED_RESET(AGE_KIVEC7_LIF,
                            AGE_KIVEC7_QTYPE)
    CAPRI_DMA_CMD_PHV2MEM_SETUP_STOP_FENCE_e(dma_p2m_1_dma_cmd,
                                             r_db_addr,
                                             db_data_no_index_data,
                                             db_data_no_index_data)

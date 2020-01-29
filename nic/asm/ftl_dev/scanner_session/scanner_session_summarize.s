#include "ftl_dev_shared_asm.h"

struct phv_                             p;
struct s6_tbl_k                         k;
struct s6_tbl_session_summarize_d       d;

/*
 * Registers usage
 */
#define r_stage                         r1
#define r_qstate_addr                   r2

/*
 * Registers reuse (post stage bubbling)
 */
#define r_rounds_sessions_expired       r1
#define r_expiry_map0                   r2
#define r_expiry_map1                   r3
#define r_expiry_map2                   r4
#define r_expiry_map3                   r5
#define r_bit                           r6
#define r_scratch                       r7

/*
 * Registers reuse (post expiry_maps computation)
 */
#define r_poller_qstate_addr            r_bit

/*
 * Local vars (due to registers shortage)
 */
#define l_num_expirations               d.expiry_map0

%%
    .param          session_poller_post
    .param          session_fsm_state_eval
    .param          session_scan_disable
    .align

session_summarize:

    // Bubble up to the desired stage -
    // when reached the penultimate stage, assert table lock and
    // provide a real table address to read.
    
    mfspr       r_stage, spr_mpuid
_if0:    
    seq         c1, r_stage[HW_MPU_SPR_MPUID_STAGE_SELECT], \
                    SESSION_SUMMARIZE_STAGE
    bcf         [c1], _endif0
    seq         c2, r_stage[HW_MPU_SPR_MPUID_STAGE_SELECT], \
                    SESSION_SUMMARIZE_STAGE - 1                 // delay slot
    add.e       r_qstate_addr, SESSION_KIVEC0_QSTATE_ADDR,  \
                SCANNER_SESSION_CB_TABLE_SUMMARIZE_OFFSET
    phvwrpair.c2 p.{common_te0_phv_table_lock_en...common_te0_phv_table_raw_table_size}, \
                 (TABLE_LOCK_EN << 3 | TABLE_SIZE_512_BITS), \
                 p.common_te0_phv_table_addr, r_qstate_addr     // delay slot
_endif0:

    //CLEAR_TABLE0
    sne         c1, SESSION_KIVEC0_CB_CFG_DISCARD, r0
    sne         c2, d.cb_activate, SCANNER_SESSION_CB_ACTIVATE
    bcf         [c1 | c2], _scanner_cb_cfg_discard
    add         r_rounds_sessions_expired, \
                k.{session_kivec0_round3_session3_expired...\
                   session_kivec0_round0_session0_expired}, r0  // delay slot
                                              
    // Merge all the 1's bits from r_rounds_sessions_expired to their
    // corresponding positions in the poller expiry_maps maintained in the d-vec
    
    add         r_expiry_map0, d.expiry_map0, r0
    add         r_expiry_map1, d.expiry_map1, r0
    add         r_expiry_map2, d.expiry_map2, r0
    add         r_expiry_map3, d.expiry_map3, r0
    tblwr.l     l_num_expirations, r0
_if2:
    beq         r_rounds_sessions_expired, r0, _endif2
    sub         r_bit, SESSION_KIVEC0_SESSION_ID_CURR, \
                SESSION_KIVEC8_EXPIRY_SESSION_ID_BASE           // delay slot
_loop2:    
    seq         c1, r_rounds_sessions_expired[0], 0
    bcf         [c1], _next2

    slt         c2, r_bit, HW_MPU_GPR_BITS                      // delay slot
    tbladd.l    l_num_expirations, 1    
    bcf         [c2], _next2
    fsetv.c2    r_expiry_map0, r_expiry_map0, r_bit, r_bit      // delay slot
    
    sub         r_scratch, r_bit, HW_MPU_GPR_BITS * 1
    slt         c2, r_bit, HW_MPU_GPR_BITS * 2
    bcf         [c2], _next2
    fsetv.c2    r_expiry_map1, r_expiry_map1, r_scratch, r_scratch // delay slot

    sub         r_scratch, r_bit, HW_MPU_GPR_BITS * 2
    slt         c2, r_bit, HW_MPU_GPR_BITS * 3
    bcf         [c2], _next2
    fsetv.c2    r_expiry_map2, r_expiry_map2, r_scratch, r_scratch // delay slot

    sub         r_scratch, r_bit, HW_MPU_GPR_BITS * 3
    fsetv       r_expiry_map3, r_expiry_map3, r_scratch, r_scratch
    
_next2:    
    srl         r_rounds_sessions_expired, r_rounds_sessions_expired, 1
    bne         r_rounds_sessions_expired, r0, _loop2
    add         r_bit, r_bit, 1                                 // delay slot
_endl2:    

    tblwr       d.expiry_map0, r_expiry_map0
    tblwr       d.expiry_map1, r_expiry_map1
    tblwr       d.expiry_map2, r_expiry_map2
    tblwr.f     d.expiry_map3, r_expiry_map3
_endif2:    

    SESSION_METRICS_VAL_SET(expired_entries, l_num_expirations)
    SESSION_METRICS_SET(scan_invocations)
    
    // When range_full is true, we must post to the software poller
    // even if the current expiry_maps are all zero. This so that
    // the poller can reschedule us.
    
    bbeq        SESSION_KIVEC8_SESSION_RANGE_FULL, 1, _poller_post_launch
    
    // If current expiry_maps don't have any expired sessions
    // or expiry_maps are not yet full, there's no need to post
    // (only next state eval should be done).
    
    seq         c3, r_expiry_map0, r0                           // delay slot
    seq.c3      c3, r_expiry_map1, r0
    seq.c3      c3, r_expiry_map2, r0
    seq.c3      c3, r_expiry_map3, r0
    seq         c4, SESSION_KIVEC8_EXPIRY_MAPS_FULL, 0
    bcf         [c3 | c4], _fsm_state_eval_launch

_poller_post_launch:

    // Note: expiry_maps are rearranged to little-endian for
    // sending to software poller
    phvwr       p.poller_slot_data_table_id_base, \
                SESSION_KIVEC8_EXPIRY_SESSION_ID_BASE.wx            // delay slot
    phvwr       p.poller_slot_data_expiry_map0, r_expiry_map0.dx
    phvwr       p.poller_slot_data_expiry_map1, r_expiry_map1.dx
    phvwr       p.poller_slot_data_expiry_map2, r_expiry_map2.dx
    phvwr       p.poller_slot_data_expiry_map3, r_expiry_map3.dx

    add         r_poller_qstate_addr, d.poller_qstate_addr, r0
    phvwr       p.session_kivec7_poller_qstate_addr, r_poller_qstate_addr
    LOAD_TABLE_FOR_ADDR_e(0,
                          TABLE_LOCK_EN,
                          r_poller_qstate_addr,
                          TABLE_SIZE_512_BITS,
                          session_poller_post)
                       
/*
 * Summarized result not ready for posting to software poller;
 * go evaluate next state.
 */
_fsm_state_eval_launch:
    LOAD_TABLE_NO_ADDR_e(0, session_fsm_state_eval)


/*
 * Discard due to control block configuration;
 * launch stage to disable queue scheduling
 */
_scanner_cb_cfg_discard:
 
    SESSION_METRICS_SET(cb_cfg_discards)
    LOAD_TABLE_NO_ADDR_e(0, session_scan_disable)

    

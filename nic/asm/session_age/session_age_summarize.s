#include "session_age_common.h"

struct phv_                             p;
struct s6_tbl_k                         k;
struct s6_tbl_age_summarize_d           d;

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
    .param          age_poller_post
    .param          age_fsm_state_eval
    .param          age_scan_disable
    .align

age_summarize:

    // Bubble up to the desired stage -
    // when reached the penultimate stage, assert table lock and
    // provide a real table address to read.
    
    mfspr       r_stage, spr_mpuid
_if0:    
    seq         c1, r_stage[CAPRI_SPR_MPUID_STAGE_SELECT], \
                    AGE_SUMMARIZE_STAGE
    bcf         [c1], _endif0
    seq         c2, r_stage[CAPRI_SPR_MPUID_STAGE_SELECT], \
                    AGE_SUMMARIZE_STAGE - 1                     // delay slot
    add.e       r_qstate_addr, AGE_KIVEC0_QSTATE_ADDR, \
                SESSION_AGE_CB_TABLE_SUMMARIZE_OFFSET
    phvwrpair.c2 p.{common_te0_phv_table_lock_en...common_te0_phv_table_raw_table_size}, \
                 (TABLE_LOCK_EN << 3 | TABLE_SIZE_512_BITS), \
                 p.common_te0_phv_table_addr, r_qstate_addr     // delay slot
_endif0:

    //CLEAR_TABLE0
    sne         c1, AGE_KIVEC0_CB_CFG_ERR_DISCARD, r0
    sne         c2, d.cb_activate, SESSION_AGE_CB_ACTIVATE
    bcf         [c1 | c2], _age_cb_cfg_err
    add         r_rounds_sessions_expired, \
                k.{age_kivec0_round3_session3_expired...\
                   age_kivec0_round0_session0_expired}, r0      // delay slot
                                              
    // Merge all the 1's bits from r_rounds_sessions_expired to their
    // corresponding positions in the poller expiry_maps maintained in the d-vec
    
    add         r_expiry_map0, d.expiry_map0, r0
    add         r_expiry_map1, d.expiry_map1, r0
    add         r_expiry_map2, d.expiry_map2, r0
    add         r_expiry_map3, d.expiry_map3, r0
    tblwr.l     l_num_expirations, r0
_if2:
    beq         r_rounds_sessions_expired, r0, _endif2
    sub         r_bit, AGE_KIVEC0_SESSION_ID_CURR, \
                AGE_KIVEC8_EXPIRY_SESSION_ID_BASE               // delay slot
_loop2:    
    seq         c1, r_rounds_sessions_expired[0], 0
    bcf         [c1], _next2

    slt         c2, r_bit, CAPRI_MPU_GPR_BITS                   // delay slot
    tbladd.l    l_num_expirations, 1    
    bcf         [c2], _next2
    fsetv.c2    r_expiry_map0, r_expiry_map0, r_bit, r_bit      // delay slot
    
    sub         r_scratch, r_bit, CAPRI_MPU_GPR_BITS * 1
    slt         c2, r_bit, CAPRI_MPU_GPR_BITS * 2
    bcf         [c2], _next2
    fsetv.c2    r_expiry_map1, r_expiry_map1, r_scratch, r_scratch // delay slot

    sub         r_scratch, r_bit, CAPRI_MPU_GPR_BITS * 2
    slt         c2, r_bit, CAPRI_MPU_GPR_BITS * 3
    bcf         [c2], _next2
    fsetv.c2    r_expiry_map2, r_expiry_map2, r_scratch, r_scratch // delay slot

    sub         r_scratch, r_bit, CAPRI_MPU_GPR_BITS * 3
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

    AGE_METRICS_VAL_SET(expired_sessions, l_num_expirations)
    AGE_METRICS_SET(scan_invocations)
    
    // When range_full is true, we must post to the software poller
    // even if the current expiry_maps are all zero. This so that
    // the poller can reschedule us.
    
    bbeq        AGE_KIVEC8_SESSION_RANGE_FULL, 1, _poller_post_launch
    
    // If current expiry_maps don't have any expired sessions
    // or expiry_maps are not yet full, there's no need to post
    // (only next state eval should be done).
    
    seq         c3, r_expiry_map0, r0                           // delay slot
    seq.c3      c3, r_expiry_map1, r0
    seq.c3      c3, r_expiry_map2, r0
    seq.c3      c3, r_expiry_map3, r0
    seq         c4, AGE_KIVEC8_EXPIRY_MAPS_FULL, 0
    bcf         [c3 | c4], _fsm_state_eval_launch

_poller_post_launch:

    // Note: expiry_maps are rearranged to little-endian for
    // sending to software poller
    phvwr       p.poller_slot_data_session_id_base, \
                AGE_KIVEC8_EXPIRY_SESSION_ID_BASE.wx            // delay slot
    phvwr       p.poller_slot_data_expiry_map0, r_expiry_map0.dx
    phvwr       p.poller_slot_data_expiry_map1, r_expiry_map1.dx
    phvwr       p.poller_slot_data_expiry_map2, r_expiry_map2.dx
    phvwr       p.poller_slot_data_expiry_map3, r_expiry_map3.dx

    add         r_poller_qstate_addr, d.poller_qstate_addr, r0
    phvwr       p.age_kivec7_poller_qstate_addr, r_poller_qstate_addr
    LOAD_TABLE_FOR_ADDR_e(0,
                          TABLE_LOCK_EN,
                          r_poller_qstate_addr,
                          TABLE_SIZE_512_BITS,
                          age_poller_post)
                       
/*
 * Summarized result not ready for posting to software poller;
 * go evaluate next state.
 */
_fsm_state_eval_launch:
    LOAD_TABLE_NO_ADDR_e(0, age_fsm_state_eval)


/*
 * CB not ready or has config errors;
 * launch stage to disable queue scheduling
 */
_age_cb_cfg_err:
 
    AGE_METRICS_SET(cb_cfg_err_discards)
    LOAD_TABLE_NO_ADDR_e(0, age_scan_disable)

    

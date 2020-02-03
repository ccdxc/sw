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
#define r_expiry_sub_map                r1
#define r_expiry_map0                   r2
#define r_expiry_map1                   r3
#define r_expiry_map2                   r4
#define r_expiry_map3                   r5
#define r_expiry_map_bit_pos            r6
#define r_poller_qstate_addr            r7

%%
    .param          session_poller_empty_post_eval
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
    add         r_expiry_sub_map, \
                k.{session_kivec0_round3_session3_expired...\
                   session_kivec0_round0_session0_expired}, r0  // delay slot
                                              
    // Merge all the 1's bits from r_rounds_sessions_expired to their
    // corresponding positions in the poller expiry_maps maintained in the d-vec
    
    add         r_expiry_map0, d.expiry_map0, r0
    add         r_expiry_map1, d.expiry_map1, r0
    add         r_expiry_map2, d.expiry_map2, r0
    add         r_expiry_map3, d.expiry_map3, r0
    
_if2:    
    beq         r_expiry_sub_map, r0, _endif2
    add         r_expiry_map_bit_pos, SESSION_KIVEC8_EXPIRY_MAP_BIT_POS, r0     // delay slot

_switch0:
    slt         c1, r_expiry_map_bit_pos, HW_MPU_GPR_BITS * 1
    bcf         [!c1], _case_try_map1 
    sll.c1      r_expiry_sub_map, r_expiry_sub_map, r_expiry_map_bit_pos // delay slot
    b           _endsw0
    or          r_expiry_map0, r_expiry_map0, r_expiry_sub_map  // delay slot
_case_try_map1:
    slt         c1, r_expiry_map_bit_pos, HW_MPU_GPR_BITS * 2
    bcf         [!c1], _case_try_map2
    sub.c1      r_expiry_map_bit_pos, r_expiry_map_bit_pos, HW_MPU_GPR_BITS * 1 // delay slot
    sll         r_expiry_sub_map, r_expiry_sub_map, r_expiry_map_bit_pos
    b           _endsw0
    or          r_expiry_map1, r_expiry_map1, r_expiry_sub_map  // delay slot
_case_try_map2:
    slt         c1, r_expiry_map_bit_pos, HW_MPU_GPR_BITS * 3
    bcf         [!c1], _case_try_map3
    sub.c1      r_expiry_map_bit_pos, r_expiry_map_bit_pos, HW_MPU_GPR_BITS * 2 // delay slot
    sll         r_expiry_sub_map, r_expiry_sub_map, r_expiry_map_bit_pos
    b           _endsw0
    or          r_expiry_map2, r_expiry_map2, r_expiry_sub_map  // delay slot
_case_try_map3:

    // This is the expected default case
    
    sub         r_expiry_map_bit_pos, r_expiry_map_bit_pos, HW_MPU_GPR_BITS * 3
    sll         r_expiry_sub_map, r_expiry_sub_map, r_expiry_map_bit_pos
    or          r_expiry_map3, r_expiry_map3, r_expiry_sub_map
_endsw0:

    tblwr       d.expiry_map0, r_expiry_map0
    tblwr       d.expiry_map1, r_expiry_map1
    tblwr       d.expiry_map2, r_expiry_map2
    tblwr.f     d.expiry_map3, r_expiry_map3
_endif2:    

    SESSION_METRICS_SET(scan_invocations)
    
    // Note: expiry_maps are rearranged to little-endian for
    // sending to software poller
    phvwr       p.poller_slot_data_table_id_base, \
                SESSION_KIVEC8_EXPIRY_ID_BASE.wx
    phvwr       p.poller_slot_data_expiry_map0, r_expiry_map0.dx
    phvwr       p.poller_slot_data_expiry_map1, r_expiry_map1.dx
    phvwr       p.poller_slot_data_expiry_map2, r_expiry_map2.dx
    phvwr       p.poller_slot_data_expiry_map3, r_expiry_map3.dx

    add         r_poller_qstate_addr, d.poller_qstate_addr, r0
    phvwr       p.session_kivec7_poller_qstate_addr, r_poller_qstate_addr

    // Evaluate whether current expiry maps are all empty
    
    seq         c3, r_expiry_map0, r0
    seq.c3      c3, r_expiry_map1, r0
    seq.c3      c3, r_expiry_map2, r0
    seq.c3      c3, r_expiry_map3, r0

    // If current maps are empty but range_full is also true, we may still have
    // to post anyway (if some non-zero maps had been previously posted for
    // the same range) so poller can reschedule us.
        
    bbeq.c3     SESSION_KIVEC8_RANGE_FULL, 1, _empty_post_eval_launch
    phvwr       p.session_kivec8_range_has_posted, d.range_has_posted // delay slot
    
    // Range_full with non-empty expiry_maps, always post
    
    bbeq        SESSION_KIVEC8_RANGE_FULL, 1, _poller_post_launch
    
    // If current expiry_maps don't have any expired sessions
    // or expiry_maps are not yet full, there's no need to post
    // (only next state eval should be done).
    
    seq         c4, SESSION_KIVEC8_EXPIRY_MAPS_FULL, 0          // delay slot
    bcf         [c3 | c4], _fsm_state_eval_launch
    nop

_poller_post_launch:
    
    // This point is reached when
    //   - expiry_maps_full=1 and they contain some expired session bits, or
    //   - range_full=1 and current expiry maps are non-empty
    // These maps will be posted to poller
    
    LOAD_TABLE_FOR_ADDR_e(0,
                          TABLE_LOCK_EN,
                          r_poller_qstate_addr,
                          TABLE_SIZE_512_BITS,
                          session_poller_post)
                          
_empty_post_eval_launch:

    // Launch next stage to evaluate whether it's necessary to post even
    // when the current expiry maps are empty. Note that we reached this
    // decision point because range_full=1 was also one of the conditions.
    
    LOAD_TABLE_FOR_ADDR_e(0,
                          TABLE_LOCK_EN,
                          r_poller_qstate_addr,
                          TABLE_SIZE_512_BITS,
                          session_poller_empty_post_eval)
                       
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

    

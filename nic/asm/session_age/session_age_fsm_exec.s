#include "session_age_common.h"

struct phv_                             p;
struct s1_tbl_k                         k;
struct s1_tbl_age_fsm_exec_d            d;

/*
 * Registers usage
 */
#define r_session_id_next               r1
#define r_total_sessions_scanned        r2
#define r_expiry_session_id_base        r3
#define r_expiry_map_sessions_scanned   r4
#define r_session_info_addr             r5
#define r_num_scannables                r6
#define r_scratch                       r7

/*
 * Registers reuse (post expiry_session_id_base computation)
 */
#define r_limit                         r_expiry_session_id_base 
 
/*
 * Registers reuse (for _resume_summarize)
 */
#define r_stage                         r6
#define r_qstate_addr                   r7
 
%%
    .param          age_round0_session0
    .param          age_round0_session1
    .param          age_round0_session2
    .param          age_round0_session3
    .param          age_summarize
    .align

age_fsm_exec:

    //CLEAR_TABLE0
    sne         c1, d.cb_activate, SESSION_AGE_CB_ACTIVATE

    // Validate parameters
    seq         c2, d.session_range_sz, r0
    slt         c3, d.session_range_sz, d.total_sessions_scanned
    slt         c4, d.session_range_sz, d.expiry_map_sessions_scanned
    bcf         [c1 | c2 | c3 | c4], _age_cb_cfg_err
    
    add         r_total_sessions_scanned, d.total_sessions_scanned, r0 // delay slot
    add         r_expiry_map_sessions_scanned, d.expiry_map_sessions_scanned, r0
    add         r_expiry_session_id_base, d.expiry_session_id_base, r0
    phvwr       p.age_kivec0_session_table_addr, d.session_table_base_addr

    add         r_scratch, d.fsm_state, r0
_switch0:    
  .brbegin
    br          r_scratch[1:0]
    add         r_session_id_next, d.session_id_next, r0    // delay slot

  .brcase SESSION_AGE_STATE_RESTART_RANGE
  
    tblwr       d.session_id_next, d.session_id_base
    tblwr       d.total_sessions_scanned, r0
    add         r_session_id_next, d.session_id_base, r0

    // There's no "fall through" with MPU assembly
    
    b           _expiry_map_restart
    add         r_total_sessions_scanned, r0, r0            // delay slot
    
  .brcase SESSION_AGE_STATE_RESTART_EXPIRY_MAP
  
_expiry_map_restart:

    tblwr       d.expiry_map_sessions_scanned, r0
    add         r_expiry_map_sessions_scanned, r0, r0
    b           _reevaluate
    add         r_expiry_session_id_base, d.session_id_next, r0 // delay slot
    
  .brcase SESSION_AGE_STATE_SCAN
  
    b           _reevaluate
    nop
    
  .brcase SESSION_AGE_STATE_REEVALUATE

_reevaluate:

    phvwr       p.age_kivec0_session_id_curr, r_session_id_next
    tblwr       d.expiry_session_id_base, r_expiry_session_id_base
    phvwr       p.age_kivec8_expiry_session_id_base, r_expiry_session_id_base

    // Calculate # of sessions that are readable across the usable stages
    // and the 4 table engines.

    // IMPORTANT NOTE: evaluation of session_range_full, expiry_maps_full,
    // and session_batch_full must always take place to ensure we correctly
    // pick up from where we last left off (in the case of resumption due
    // to a previous poller queue full condition).
        
    sub         r_num_scannables, d.session_range_sz, r_total_sessions_scanned
    sle         c1, AGE_SESSIONS_MPU_TABLES_TOTAL, r_num_scannables
    add.c1      r_num_scannables, r0, AGE_SESSIONS_MPU_TABLES_TOTAL

    // Hard limit # of scans to the expiry maps limit
    sub         r_limit, SESSION_AGE_EXPIRY_MAP_ENTRIES_TOTAL_BITS, \
                r_expiry_map_sessions_scanned
    sle         c2, r_limit, r_num_scannables
    add.c2      r_num_scannables, r0, r_limit
    
    // Limit # of scans to the batch limit;
    // Note this is a soft limit so it can just be calculated from
    // the current session ID without keeping any more state info.
_if2:    
    seq         c3, d.session_batch_sz, r0
    bcf         [c3], _endif2
    add         r_limit, r_session_id_next, r0          // delay slot
    mincr       r_limit, d.session_batch_sz_shft, r0
    sub         r_limit, d.session_batch_sz, r_limit
    sle         c3, r_limit, r_num_scannables
    add.c3      r_num_scannables, r0, r_limit
    phvwr.c3    p.age_kivec8_session_batch_full, 1
_endif2:
    
    // Update certain state info for next PHV resumption
    add         r_total_sessions_scanned, r_total_sessions_scanned, \
                r_num_scannables
    sle         c4, d.session_range_sz, r_total_sessions_scanned
    phvwr.c4    p.age_kivec8_session_range_full, 1
    tblwr       d.total_sessions_scanned, r_total_sessions_scanned
    
    add         r_expiry_map_sessions_scanned, r_expiry_map_sessions_scanned, \
                r_num_scannables
    sle         c5, SESSION_AGE_EXPIRY_MAP_ENTRIES_TOTAL_BITS, \
                r_expiry_map_sessions_scanned
    phvwr.c5    p.age_kivec8_expiry_maps_full, 1
    tblwr       d.expiry_map_sessions_scanned, r_expiry_map_sessions_scanned
    
    // Issue session info read scans
    beq         r_num_scannables, r0, _resume_summarize
    add         r_session_info_addr, d.session_table_base_addr, \
                r_session_id_next, SESSION_INFO_BYTES_SHFT      // delay slot
    AGE_SESSION_INFO_POSSIBLE_SCAN_INCR(0, AGE_SESSION_ROUND0,
                                        r_num_scannables, age_round0_session0, 
                                        _scan_break)
    AGE_SESSION_INFO_POSSIBLE_SCAN_INCR(1, AGE_SESSION_ROUND0,
                                        r_num_scannables, age_round0_session1,
                                        _scan_break)
    AGE_SESSION_INFO_POSSIBLE_SCAN_INCR(2, AGE_SESSION_ROUND0,
                                        r_num_scannables, age_round0_session2,
                                        _scan_break)
    AGE_SESSION_INFO_POSSIBLE_SCAN_INCR(3, AGE_SESSION_ROUND0,
                                        r_num_scannables, age_round0_session3,
                                        _scan_break)
_scan_break:
    phvwr.e     p.age_kivec0_num_scannables, r_num_scannables
    tbladd.f    d.session_id_next, r_num_scannables

  .brend
_endsw0:

/*
 * CB not ready or has config errors
 */
_age_cb_cfg_err:
 
    AGE_METRICS_SET(cb_cfg_err_discards)
    phvwr       p.age_kivec0_cb_cfg_err_discard, 1
    
/*
 * Possible reasons to have reached this point are:
 * - Poller queue full detected during completion processing in the
 *   previously scheduled scan
 * - CB config errors (see above)
 */
_resume_summarize:

    //
    // Here we will attempt to resume from the summarize step. See also
    // "IMPORTANT NOTE" above regarding the re-evaluation of various
    // state such as session_range_full, expiry_maps_full, etc.
         
    AGE_SUMMARIZE_LAUNCH_e(0, AGE_KIVEC0_QSTATE_ADDR, age_summarize)


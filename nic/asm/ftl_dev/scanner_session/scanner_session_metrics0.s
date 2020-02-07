#include "ftl_dev_shared_asm.h"

struct phv_                              p;
struct s7_tbl3_k                         k;
struct s7_tbl3_session_metrics0_commit_d d;

/*
 * Registers usage
 */
#define r_range_elapsed_ticks           r1
#define r_stage                         r3
#define r_qstate_addr                   r4
#define r_expired_entries               r5

    
%%
    .align

session_metrics0_commit:

    // Bubble up to the desired stage -
    // when reached the penultimate stage, assert table lock and
    // provide a real table address to read.
    
    mfspr       r_stage, spr_mpuid
_if0:    
    seq         c1, r_stage[HW_MPU_SPR_MPUID_STAGE_SELECT], \
                    SESSION_METRICS_STAGE
    bcf         [c1], _endif0
    seq         c2, r_stage[HW_MPU_SPR_MPUID_STAGE_SELECT], \
                    SESSION_METRICS_STAGE - 1                   // delay slot
    add.e       r_qstate_addr, SESSION_KIVEC0_QSTATE_ADDR, \
                SCANNER_SESSION_CB_TABLE_METRICS0_OFFSET
    phvwrpair.c2 p.{common_te3_phv_table_lock_en...common_te3_phv_table_raw_table_size}, \
                 (TABLE_LOCK_EN << 3 | TABLE_SIZE_512_BITS), \
                 p.common_te3_phv_table_addr, r_qstate_addr     // delay slot
_endif0:

    seq         c3, SESSION_KIVEC9_METRICS0_RANGE, r0
    seq.c3      c3, SESSION_KIVEC0_ROUNDS_SESSIONS_RANGE, r0

    // common_txdma sets up stage7 table3 without phvwr capability
    // so we don't clear table3 here to avoid a phvwr error.
    
    nop.c3.e
    add         r_expired_entries, SESSION_KIVEC0_ROUND3_SESSION3, \
                                   SESSION_KIVEC0_ROUND3_SESSION2 // delay slot
    
    SCANNER_METRICS_TBLADD(cb_cfg_discards,   SESSION_KIVEC9_CB_CFG_DISCARDS)
    SCANNER_METRICS_TBLADD(scan_invocations,  SESSION_KIVEC9_SCAN_INVOCATIONS)

    // Calculate min/max elapsed ticks for the scan range    
_if2:
    add         r_range_elapsed_ticks, SESSION_KIVEC9_RANGE_ELAPSED_TICKS, r0
    beq         r_range_elapsed_ticks, r0, _endif2
    slt         c4, r_range_elapsed_ticks, d.min_range_elapsed_ticks // delay slot
    tblwr.c4    d.min_range_elapsed_ticks, r_range_elapsed_ticks
    slt         c5, d.max_range_elapsed_ticks, r_range_elapsed_ticks
    tblwr.c5    d.max_range_elapsed_ticks, r_range_elapsed_ticks
_endif2:    

    // Note: SESSION_KIVEC0_ROUND3_SESSION3 and SESSION_KIVEC0_ROUND3_SESSION3
    // already accounted for above.
    
    //add       r_expired_entries, r_expired_entries, SESSION_KIVEC0_ROUND3_SESSION3
    //add       r_expired_entries, r_expired_entries, SESSION_KIVEC0_ROUND3_SESSION2
    add         r_expired_entries, r_expired_entries, SESSION_KIVEC0_ROUND3_SESSION1
    add         r_expired_entries, r_expired_entries, SESSION_KIVEC0_ROUND3_SESSION0
    add         r_expired_entries, r_expired_entries, SESSION_KIVEC0_ROUND2_SESSION3
    add         r_expired_entries, r_expired_entries, SESSION_KIVEC0_ROUND2_SESSION2
    add         r_expired_entries, r_expired_entries, SESSION_KIVEC0_ROUND2_SESSION1
    add         r_expired_entries, r_expired_entries, SESSION_KIVEC0_ROUND2_SESSION0
    add         r_expired_entries, r_expired_entries, SESSION_KIVEC0_ROUND1_SESSION3
    add         r_expired_entries, r_expired_entries, SESSION_KIVEC0_ROUND1_SESSION2
    add         r_expired_entries, r_expired_entries, SESSION_KIVEC0_ROUND1_SESSION1
    add         r_expired_entries, r_expired_entries, SESSION_KIVEC0_ROUND1_SESSION0
    add         r_expired_entries, r_expired_entries, SESSION_KIVEC0_ROUND0_SESSION3
    add         r_expired_entries, r_expired_entries, SESSION_KIVEC0_ROUND0_SESSION2
    add         r_expired_entries, r_expired_entries, SESSION_KIVEC0_ROUND0_SESSION1
    add         r_expired_entries, r_expired_entries, SESSION_KIVEC0_ROUND0_SESSION0
    SCANNER_METRICS_TBLADD_e(expired_entries, r_expired_entries)


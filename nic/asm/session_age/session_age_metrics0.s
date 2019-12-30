#include "session_age_common.h"

struct phv_                             p;
struct s7_tbl3_k                        k;
struct s7_tbl3_age_metrics0_commit_d    d;

/*
 * Registers usage
 */
#define r_stage                         r3
#define r_qstate_addr                   r4

    
%%
    .align

age_metrics0_commit:

    // Bubble up to the desired stage -
    // when reached the penultimate stage, assert table lock and
    // provide a real table address to read.
    
    mfspr       r_stage, spr_mpuid
_if0:    
    seq         c1, r_stage[CAPRI_SPR_MPUID_STAGE_SELECT], \
                    AGE_METRICS_STAGE
    bcf         [c1], _endif0
    seq         c2, r_stage[CAPRI_SPR_MPUID_STAGE_SELECT], \
                    AGE_METRICS_STAGE - 1 // delay slot
    add.e       r_qstate_addr, AGE_KIVEC0_QSTATE_ADDR, \
                SESSION_AGE_CB_TABLE_METRICS0_OFFSET
    phvwrpair.c2 p.{common_te3_phv_table_lock_en...common_te3_phv_table_raw_table_size}, \
                 (TABLE_LOCK_EN << 3 | TABLE_SIZE_512_BITS), \
                 p.common_te3_phv_table_addr, r_qstate_addr     // delay slot
_endif0:

    seq         c3, AGE_KIVEC9_METRICS0_RANGE, r0
    nop.c3.e
    CLEAR_TABLE3                                                // delay slot
    
    AGE_METRICS_TBLADD(cb_cfg_err_discards, AGE_KIVEC9_CB_CFG_ERR_DISCARDS)
    AGE_METRICS_TBLADD(scan_invocations,    AGE_KIVEC9_SCAN_INVOCATIONS)
    AGE_METRICS_TBLADD_e(expired_sessions,  AGE_KIVEC9_EXPIRED_SESSIONS)


#include "ftl_dev_shared_asm.h"

struct phv_                              p;
struct s7_tbl3_k                         k;
struct s7_tbl3_session_metrics0_commit_d d;

/*
 * Registers usage
 */
#define r_stage                         r3
#define r_qstate_addr                   r4

    
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
                    SESSION_METRICS_STAGE - 1 // delay slot
    add.e       r_qstate_addr, SESSION_KIVEC0_QSTATE_ADDR, \
                SCANNER_SESSION_CB_TABLE_METRICS0_OFFSET
    phvwrpair.c2 p.{common_te3_phv_table_lock_en...common_te3_phv_table_raw_table_size}, \
                 (TABLE_LOCK_EN << 3 | TABLE_SIZE_512_BITS), \
                 p.common_te3_phv_table_addr, r_qstate_addr     // delay slot
_endif0:

    seq         c3, SESSION_KIVEC9_METRICS0_RANGE, r0

    // common_txdma sets up stage7 table3 without phvwr capability
    // so we don't clear table3 here to avoid a phvwr error.
    
    nop.c3.e
    nop
    
    SCANNER_METRICS_TBLADD(cb_cfg_discards,   SESSION_KIVEC9_CB_CFG_DISCARDS)
    SCANNER_METRICS_TBLADD(scan_invocations,  SESSION_KIVEC9_SCAN_INVOCATIONS)
    SCANNER_METRICS_TBLADD_e(expired_entries, SESSION_KIVEC9_EXPIRED_ENTRIES)


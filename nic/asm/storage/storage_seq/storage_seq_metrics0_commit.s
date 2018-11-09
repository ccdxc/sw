/*****************************************************************************
 *  seq_metrics0_commit
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s5_tbl1_k k;
struct s5_tbl1_seq_metrics0_commit_d d;
struct phv_ p;

/*
 * Registers usage:
 */
#define r_stage                     r3

%%

storage_seq_metrics0_commit:

    // bubble up to SEQ_METRICS_STAGE (table already locked) for atomic updates
    mfspr       r_stage, spr_mpuid
    slt         c1, r_stage[4:2], SEQ_METRICS_STAGE
    nop.c1.e
    seq         c2, SEQ_KIVEC9_METRICS0_RANGE, r0       // delay slot
    nop.c2.e
    CLEAR_TABLE1                                        // delay slot
    
    SEQ_METRICS_TBLADD_c(c3,   interrupts_raised, SEQ_KIVEC9_INTERRUPTS_RAISED)
    SEQ_METRICS_TBLADD_c(c3,   next_db_rung, SEQ_KIVEC9_NEXT_DB_RUNG)
    SEQ_METRICS_TBLADD_c(c3,   descs_processed, SEQ_KIVEC9_DESCS_PROCESSED)
    SEQ_METRICS_TBLADD_c(c3,   descs_aborted, SEQ_KIVEC9_DESCS_ABORTED)
    SEQ_METRICS_TBLADD_c(c3,   status_pdma_xfers, SEQ_KIVEC9_STATUS_PDMA_XFERS)
    SEQ_METRICS_TBLADD_c(c3,   hw_desc_xfers, SEQ_KIVEC9_HW_DESC_XFERS)
    SEQ_METRICS_TBLADD_c(c3,   hw_batch_errs, SEQ_KIVEC9_HW_BATCH_ERRS)
    SEQ_METRICS_TBLADD_c_e(c3, hw_op_errs, SEQ_KIVEC9_HW_OP_ERRS)


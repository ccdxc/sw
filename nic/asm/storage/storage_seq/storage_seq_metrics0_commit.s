/*****************************************************************************
 *  seq_metrics0_commit
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s7_tbl1_k k;
struct s7_tbl1_seq_metrics0_commit_d d;
struct phv_ p;

/*
 * Registers usage:
 */
#define r_stage                     r3

%%

storage_seq_metrics0_commit:

    // Bubble to common stage 7 (table already locked) for atomic updates
    mfspr       r_stage, spr_mpuid
    slt         c1, r_stage[4:2], STAGE_7
    nop.c1.e
    seq         c2, SEQ_KIVEC9_METRICS0_RANGE, r0       // delay slot
    nop.c2.e
    CLEAR_TABLE1                                        // delay slot
    
    tbladd      d.interrupts_raised, SEQ_KIVEC9_INTERRUPTS_RAISED
    tbladd      d.next_db_rung, SEQ_KIVEC9_NEXT_DB_RUNG
    tbladd      d.descs_processed, SEQ_KIVEC9_DESCS_PROCESSED
    tbladd      d.descs_aborted, SEQ_KIVEC9_DESCS_ABORTED
    tbladd      d.status_pdma_xfers, SEQ_KIVEC9_STATUS_PDMA_XFERS
    tbladd      d.hw_desc_xfers, SEQ_KIVEC9_HW_DESC_XFERS
    tbladd.e    d.hw_batch_errs, SEQ_KIVEC9_HW_BATCH_ERRS
    tbladd      d.hw_op_errs, SEQ_KIVEC9_HW_OP_ERRS


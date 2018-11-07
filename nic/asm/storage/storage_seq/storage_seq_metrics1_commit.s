/*****************************************************************************
 *  seq_metrics1_commit
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s5_tbl2_k k;
struct s5_tbl2_seq_metrics1_commit_d d;
struct phv_ p;

/*
 * Registers usage:
 */
#define r_stage                     r3

%%

storage_seq_metrics1_commit:

    // bubble up to common stage 7 (table already locked) for atomic updates
    mfspr       r_stage, spr_mpuid
    slt         c1, r_stage[4:2], SEQ_METRICS_STAGE
    nop.c1.e
    seq         c2, SEQ_KIVEC9_METRICS1_RANGE, r0       // delay slot
    nop.c2.e
    CLEAR_TABLE2                                        // delay slot
    
    SEQ_METRICS_TBLADD_c(c3,   aol_pad_reqs, SEQ_KIVEC9_AOL_PAD_REQS)
    SEQ_METRICS_TBLADD_c(c3,   sgl_pad_reqs, SEQ_KIVEC9_SGL_PAD_REQS)
    SEQ_METRICS_TBLADD_c(c3,   sgl_pdma_xfers, SEQ_KIVEC9_SGL_PDMA_XFERS)
    SEQ_METRICS_TBLADD_c(c3,   sgl_pdma_errs, SEQ_KIVEC9_SGL_PDMA_ERRS)
    SEQ_METRICS_TBLADD_c(c3,   sgl_pad_only_xfers, SEQ_KIVEC9_SGL_PAD_ONLY_XFERS)
    SEQ_METRICS_TBLADD_c(c3,   sgl_pad_only_errs, SEQ_KIVEC9_SGL_PAD_ONLY_ERRS)
    SEQ_METRICS_TBLADD_c(c3,   alt_descs_taken, SEQ_KIVEC9_ALT_DESCS_TAKEN)
    SEQ_METRICS_TBLADD_c_e(c3, alt_bufs_taken, SEQ_KIVEC9_ALT_BUFS_TAKEN)


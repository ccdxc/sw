/*****************************************************************************
 *  seq_metrics2_commit
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s5_tbl3_k k;
struct s5_tbl3_seq_metrics2_commit_d d;
struct phv_ p;

/*
 * Registers usage:
 */
#define r_stage                     r3

%%

storage_seq_metrics2_commit:

    // bubble up to SEQ_METRICS_STAGE (table already locked) for atomic updates
    mfspr       r_stage, spr_mpuid
    slt         c1, r_stage[4:2], SEQ_METRICS_STAGE
    nop.c1.e
    seq         c2, SEQ_KIVEC9_METRICS2_RANGE, r0       // delay slot
    nop.c2.e
    CLEAR_TABLE3                                        // delay slot
    
    SEQ_METRICS_TBLADD_c_e(c3, len_updates, SEQ_KIVEC9_LEN_UPDATES)



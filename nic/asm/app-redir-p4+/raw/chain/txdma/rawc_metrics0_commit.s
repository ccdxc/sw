#include "app_redir_common.h"

struct s7_tbl2_k                        k;
struct s7_tbl2_metrics0_commit_d        d;
struct phv_                             p;

/*
 * Registers usage:
 */
#define r_stage                     r3

%%
    .align

rawc_metrics0_commit:

    // bubble up to RAWC_METRICS_STAGE (table already locked) for atomic updates
    mfspr       r_stage, spr_mpuid
    slt         c1, r_stage[4:2], RAWC_METRICS_STAGE
    nop.c1.e
    
    // Mark pipeline for drop if not chaining any packets
    seq         c1, RAWC_KIVEC9_CHAIN_PKTS, r0          // delay slot
    phvwr.c1    p.p4_intr_global_drop, 1
    
    seq         c2, RAWC_KIVEC9_METRICS0_RANGE, r0
    nop.c2.e
    CAPRI_CLEAR_TABLE2_VALID    // delay slot
    
    APP_REDIR_METRICS_TBLADD_c(c3,   chain_pkts, RAWC_KIVEC9_CHAIN_PKTS)
    APP_REDIR_METRICS_TBLADD_c(c3,   cb_not_ready_discards, RAWC_KIVEC9_CB_NOT_READY_DISCARDS)
    APP_REDIR_METRICS_TBLADD_c(c3,   qstate_cfg_discards, RAWC_KIVEC9_QSTATE_CFG_DISCARDS)
    APP_REDIR_METRICS_TBLADD_c(c3,   aol_error_discards, RAWC_KIVEC9_AOL_ERROR_DISCARDS)
    APP_REDIR_METRICS_TBLADD_c(c3,   my_txq_empty_discards, RAWC_KIVEC9_MY_TXQ_EMPTY_DISCARDS)
    APP_REDIR_METRICS_TBLADD_c(c3,   txq_full_discards, RAWC_KIVEC9_TXQ_FULL_DISCARDS)
    APP_REDIR_METRICS_TBLADD_c_e(c3, pkt_free_errors, RAWC_KIVEC9_PKT_FREE_ERRORS)


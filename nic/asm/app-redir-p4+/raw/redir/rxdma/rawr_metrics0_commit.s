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

rawr_metrics0_commit:

    // bubble up to RAWR_METRICS_STAGE (table already locked) for atomic updates
    mfspr       r_stage, spr_mpuid
    slt         c1, r_stage[4:2], RAWR_METRICS_STAGE
    nop.c1.e
    
    // Mark pipeline for drop if not redirecting any packets
    seq         c1, RAWR_KIVEC9_REDIR_PKTS, r0          // delay slot
    phvwr.c1    p.p4_intr_global_drop, 1
    
    seq         c2, RAWR_KIVEC9_METRICS0_RANGE, r0
    nop.c2.e
    CAPRI_CLEAR_TABLE2_VALID    // delay slot
    
    APP_REDIR_METRICS_TBLADD_c(c3,   redir_pkts, RAWR_KIVEC9_REDIR_PKTS)
    APP_REDIR_METRICS_TBLADD_c(c3,   cb_not_ready_discards, RAWR_KIVEC9_CB_NOT_READY_DISCARDS)
    APP_REDIR_METRICS_TBLADD_c(c3,   qstate_cfg_discards, RAWR_KIVEC9_QSTATE_CFG_DISCARDS)
    APP_REDIR_METRICS_TBLADD_c(c3,   pkt_len_discards, RAWR_KIVEC9_PKT_LEN_DISCARDS)
    APP_REDIR_METRICS_TBLADD_c(c3,   rxq_full_discards, RAWR_KIVEC9_RXQ_FULL_DISCARDS)
    APP_REDIR_METRICS_TBLADD_c(c3,   txq_full_discards, RAWR_KIVEC9_TXQ_FULL_DISCARDS)
    APP_REDIR_METRICS_TBLADD_c(c3,   pkt_alloc_errors, RAWR_KIVEC9_PKT_ALLOC_ERRORS)
    APP_REDIR_METRICS_TBLADD_c_e(c3, pkt_free_errors, RAWR_KIVEC9_PKT_FREE_ERRORS)


#include "app_redir_common.h"

struct phv_                             p;
struct rawr_stats_k                     k;
struct rawr_stats_err_stat_inc_d        d;

/*
 * Registers usage
 */

%%
    
    .align

/*
 * Non-atomic increment of a 32-bit saturating error counter.
 * In addition, a 64-bit packet discard counter is also incremented.
 *
 * Caution: function is stage agnostic, but must not be launched from stage 7!
 */
rawr_err_stats_inc:

    seq         c1, k.t3_s2s_stat_byte_offs, RAWRCB_STAT_CB_NOT_READY_BYTE_OFFS
    tblsadd.c1  d.stat_cb_not_ready, 1
    seq         c1, k.t3_s2s_stat_byte_offs, RAWRCB_STAT_QSTATE_CFG_ERR_BYTE_OFFS
    tblsadd.c1  d.stat_qstate_cfg_err, 1
    seq         c1, k.t3_s2s_stat_byte_offs, RAWRCB_STAT_PKT_LEN_ERR_BYTE_OFFS
    tblsadd.c1  d.stat_pkt_len_err, 1
    seq         c1, k.t3_s2s_stat_byte_offs, RAWRCB_STAT_RXQ_FULL_BYTE_OFFS
    tblsadd.c1  d.stat_rxq_full, 1
    seq         c1, k.t3_s2s_stat_byte_offs, RAWRCB_STAT_TXQ_FULL_BYTE_OFFS
    tblsadd.c1  d.stat_txq_full, 1
    seq         c1, k.t3_s2s_stat_byte_offs, RAWRCB_STAT_SEM_ALLOC_FULL_BYTE_OFFS
    tblsadd.c1  d.stat_sem_alloc_full, 1
    seq         c1, k.t3_s2s_stat_byte_offs, RAWRCB_STAT_SEM_FREE_FULL_BYTE_OFFS
    tblsadd.c1  d.stat_sem_free_full, 1
    
    phvwri.e    p.app_header_table3_valid, 0
    tbladd.c1   d.stat_pkts_discard, 1  // delay slot

    .align

/*
 * Non-atomic increment of a single counter.
 *
 * Caution: function is stage agnostic, but must not be launched from stage 7!
 */
rawr_single_stat_inc:

    phvwri      p.app_header_table3_valid, 0
    seq.e       c1, k.t3_s2s_stat_byte_offs, RAWRCB_STAT_PKTS_REDIR_BYTE_OFFS
    tbladd.c1   d.stat_pkts_redir, 1    // delay slot


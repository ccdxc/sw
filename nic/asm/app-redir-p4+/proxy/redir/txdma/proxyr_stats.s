#include "app_redir_common.h"

struct phv_                             p;
struct proxyr_stats_k                   k;
struct proxyr_stats_err_stat_inc_d      d;

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
proxyr_err_stats_inc:

    seq         c1, k.t3_s2s_stat_byte_offs, PROXYRCB_STAT_CB_NOT_READY_BYTE_OFFS
    tblsadd.c1  d.stat_cb_not_ready, 1
    seq         c1, k.t3_s2s_stat_byte_offs, PROXYRCB_STAT_NULL_RING_INDICES_ADDR_BYTE_OFFS
    tblsadd.c1  d.stat_null_ring_indices_addr, 1
    seq         c1, k.t3_s2s_stat_byte_offs, PROXYRCB_STAT_AOL_ERR_BYTE_OFFS
    tblsadd.c1  d.stat_aol_err, 1
    seq         c1, k.t3_s2s_stat_byte_offs, PROXYRCB_STAT_RXQ_FULL_BYTE_OFFS
    tblsadd.c1  d.stat_rxq_full, 1
    seq         c1, k.t3_s2s_stat_byte_offs, PROXYRCB_STAT_TXQ_EMPTY_BYTE_OFFS
    tblsadd.c1  d.stat_txq_empty, 1
    seq         c1, k.t3_s2s_stat_byte_offs, PROXYRCB_STAT_SEM_ALLOC_FULL_BYTE_OFFS
    tblsadd.c1  d.stat_sem_alloc_full, 1
    seq         c1, k.t3_s2s_stat_byte_offs, PROXYRCB_STAT_SEM_FREE_FULL_BYTE_OFFS
    tblsadd.c1  d.stat_sem_free_full, 1
    
    phvwri.e    p.app_header_table3_valid, 0
    tbladd.c1   d.stat_pkts_discard, 1  // delay slot

    .align

/*
 * Non-atomic increment of a single counter.
 *
 * Caution: function is stage agnostic, but must not be launched from stage 7!
 */
proxyr_single_stat_inc:

    phvwri      p.app_header_table3_valid, 0
    seq.e       c1, k.t3_s2s_stat_byte_offs, PROXYRCB_STAT_PKTS_REDIR_BYTE_OFFS
    tbladd.c1   d.stat_pkts_redir, 1    // delay slot


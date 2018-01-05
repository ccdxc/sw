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
 * Non-atomic increment of a 32-bit saturating error counters.
 * In addition, a 64-bit packet discard counter is also incremented.
 *
 * Caution: function is stage agnostic, but must not be launched from stage 7!
 */
rawr_err_stats_inc:

    CAPRI_CLEAR_TABLE3_VALID
    
    tblsadd     d.stat_cb_not_ready,  k.t3_s2s_inc_stat_cb_not_ready
    tblsadd     d.stat_qstate_cfg_err, k.t3_s2s_inc_stat_qstate_cfg_err
    tblsadd     d.stat_pkt_len_err, k.t3_s2s_inc_stat_pkt_len_err
    tblsadd     d.stat_rxq_full, k.t3_s2s_inc_stat_rxq_full
    tblsadd     d.stat_txq_full, k.t3_s2s_inc_stat_txq_full
    tblsadd     d.stat_desc_sem_alloc_full, k.t3_s2s_inc_stat_desc_sem_alloc_full
    tblsadd     d.stat_mpage_sem_alloc_full, k.t3_s2s_inc_stat_mpage_sem_alloc_full
    tblsadd     d.stat_ppage_sem_alloc_full, k.t3_s2s_inc_stat_ppage_sem_alloc_full
    tblsadd     d.stat_sem_free_full, k.t3_s2s_inc_stat_sem_free_full

    sne         c1, k.{t3_s2s_inc_stat_begin...t3_s2s_inc_stat_end}, r0
    phvwri.e    p.{t3_s2s_inc_stat_begin...t3_s2s_inc_stat_end}, 0
    tbladd.c1   d.stat_pkts_discard, 1  // delay slot
    
    .align

/*
 * Non-atomic increment of normal (good) counters.
 *
 * Caution: function is stage agnostic, but must not be launched from stage 7!
 */
rawr_normal_stats_inc:

    CAPRI_CLEAR_TABLE3_VALID
    
    tbladd.e    d.stat_pkts_redir, k.t3_s2s_inc_stat_pkts_redir
    phvwri      p.{t3_s2s_inc_stat_begin...t3_s2s_inc_stat_end}, 0 // delay slot


#include "app_redir_common.h"

struct phv_                         p;
struct proxyr_desc_free_k           k;
struct proxyr_desc_free_desc_free_d d;

/*
 * Registers usage
 */
#define r_table_base                r1  // RNMDR_TABLE_BASE
#define r_table_idx                 r2  // PI index
#define r_qstate_addr               r3

%%
    .param      RNMDR_TABLE_BASE
    .param      proxyr_err_stats_inc
    
    .align

/*
 * Entered after having acquired a descriptor free index semaphore,
 * this function invokes a common function to free page 0.
 */    
proxyr_s6_desc_free:

    CAPRI_CLEAR_TABLE0_VALID

    /*
     * descriptor free semaphore should never be full
     */
    sne         c1, d.pindex_full, r0
    bcf         [c1], _err_stats_collect
    phvwri.c1   p.t3_s2s_inc_stat_sem_free_full, 1  // delay slot
    
    APP_REDIR_IMM64_LOAD(r_table_base, RNMDR_TABLE_BASE)
    add         r_table_idx, r0, d.{pindex}.wx
    mincr       r_table_idx, ASIC_RNMDR_RING_SHIFT, r0
    add         r_table_base, r_table_base, r_table_idx, \
                RNMDR_TABLE_ENTRY_SIZE_SHFT
    memwr.d     r_table_base, k.{to_s6_desc_sbit0_ebit31...\
                                 to_s6_desc_sbit32_ebit33}
    
    /*
     * Update CI
     */
    addi        r_table_base, r0, ASIC_SEM_RNMDR_ALLOC_CI_RAW_ADDR
    mincr       r_table_idx, ASIC_RNMDR_RING_SHIFT, 1
    memwr.w     r_table_base, r_table_idx.wx
    wrfence

_err_stats_collect:

    /*
     * Common point in the cleanup process for error stats collection
     */    
    PROXYRCB_ERR_STAT_INC_LAUNCH(3, r_qstate_addr,
                                 k.{common_phv_qstate_addr_sbit0_ebit5... \
                                    common_phv_qstate_addr_sbit30_ebit33},
                                 p.t3_s2s_inc_stat_check_all)
    nop.e
    nop


#include "app_redir_common.h"

struct phv_                                     p;
struct rawc_cleanup_discard_k                   k;

/*
 * Registers usage
 */
#define r_free_inf_addr             r1

%%
    .param      rawc_s5_desc_free
    .param      rawc_s5_page0_free
    .param      rawc_s5_page0_no_free
    
    .align

/*
 * Common code to lanuch cleanup code to free desc and pages
 * due to CB not ready or other error conditions.
 */
rawc_s4_cleanup_discard:

    CAPRI_CLEAR_TABLE1_VALID
        
    /*
     * Launch desc semaphore free pindex free acquisition
     */
    addi        r_free_inf_addr, r0, CAPRI_SEM_RNMDR_FREE_INF_ADDR
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                          rawc_s5_desc_free,
                          r_free_inf_addr,
                          TABLE_SIZE_64_BITS)
    /*
     * Launch page 0 semaphore free pindex free acquisition
     * if applicable
     */
    sne         c1, r0, k.{t1_s2s_aol_A0_sbit0_ebit47...\
                           t1_s2s_aol_A0_sbit48_ebit51}
    bcf         [c1], _page0_sem_free_idx_launch
    nop

    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(1, rawc_s5_page0_no_free)
    b           _discard_stats_update
    nop

_page0_sem_free_idx_launch:

    /*
     * cmov can't handle large immediate so use 2 adds
     */
    sne         c1, k.t1_s2s_aol_A0_small, r0
    addi.c1     r_free_inf_addr, r0, CAPRI_SEM_RNMPR_SMALL_FREE_INF_ADDR
    addi.!c1    r_free_inf_addr, r0, CAPRI_SEM_RNMPR_FREE_INF_ADDR
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS,
                          rawc_s5_page0_free,
                          r_free_inf_addr,
                          TABLE_SIZE_64_BITS)
                          
_discard_stats_update:
    
    /*
     * TODO: add stats here
     */
    nop.e
    nop


#include "app_redir_common.h"

struct phv_                                     p;
struct proxyc_cleanup_discard_k                 k;
struct proxyc_cleanup_discard_cleanup_discard_d d;

/*
 * Registers usage
 *
 * Note that CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP uses r1/r2 as scratch registers!
 */
#define r_free_inf_addr             r3

%%
    .param      proxyc_s5_desc_free
    .param      proxyc_s5_page0_free
    .param      proxyc_s5_page0_no_free
    
    .align

/*
 * Common code to lanuch cleanup code to free desc/ppage/mpage
 * due to semaphore pindex full on one or more such resources.
 */
proxyc_s4_cleanup_discard:

    CAPRI_CLEAR_TABLE1_VALID
        
    /*
     * Launch desc semaphore free pindex free acquisition
     */
    addi        r_free_inf_addr, r0, ASIC_SEM_RNMDR_FREE_INF_ADDR
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                          proxyc_s5_desc_free,
                          r_free_inf_addr,
                          TABLE_SIZE_64_BITS)
    /*
     * Launch page 0 semaphore free pindex free acquisition
     * if applicable
     */
    phvwr       p.t1_s2s_aol_A0, d.A0
    phvwr       p.t1_s2s_aol_A1, d.A1
    sne         c1, d.A0, r0
    bcf         [c1], _page0_sem_free_idx_launch
    phvwr       p.t1_s2s_aol_A2, d.A2   // delay slot

    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(1, proxyc_s5_page0_no_free)

    /*
     * Note that relevant statistics have already been incremented
     * prior to entering this module.
     */
    nop.e
    nop

_page0_sem_free_idx_launch:

    /*
     * cmov can't handle large immediate so use 2 adds
     */
    sne         c1, k.t1_s2s_aol_A0_small, r0
    addi.c1     r_free_inf_addr, r0, ASIC_SEM_RNMPR_SMALL_FREE_INF_ADDR
    addi.!c1    r_free_inf_addr, r0, ASIC_SEM_RNMPR_FREE_INF_ADDR
    CAPRI_NEXT_TABLE_READ_e(1, TABLE_LOCK_DIS,
                            proxyc_s5_page0_free,
                            r_free_inf_addr,
                            TABLE_SIZE_64_BITS)
    nop


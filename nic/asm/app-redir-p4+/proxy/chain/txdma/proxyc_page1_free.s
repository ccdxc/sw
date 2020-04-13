#include "app_redir_common.h"

struct phv_                             p;
struct proxyc_page1_free_k              k;
struct proxyc_page1_free_page1_free_d   d;

/*
 * Registers usage; must match same definitions in proxyc_page0_free.s
 */
#define r_page_addr                 r1  // page address to free
#define r_page_is_small             r2  // small page indicator
#define r_table_base                r3  // RNMPR_TABLE_BASE or RNMPR_SMALL_TABLE_BASE
#define r_table_idx                 r4  // PI index
#define r_return                    r5  // return address
#define r_scratch                   r6
 
%%
    .param      proxyc_s7_page2_free
    .param      proxyc_page_free
    .param      proxyc_page_free_sem_pindex_full
    
    .align

/*
 * Entered as a stage transition to launch a page 2 free index
 * semaphore acquisition when there was no page 1 to free.
 */    
proxyc_s6_page1_no_free:

    CAPRI_CLEAR_TABLE1_VALID
    
_page2_free_applic_check:

    /*
     * Launch page 2 semaphore free pindex free acquisition
     * if applicable
     */
    seq         c1, r0, k.{t1_s2s_aol_A2_sbit0_ebit47...\
                           t1_s2s_aol_A2_sbit48_ebit51}
    nop.c1.e
    sne         c1, k.t1_s2s_aol_A2_small, r0               // delay slot
    addi.c1     r_scratch, r0, ASIC_SEM_RNMPR_SMALL_FREE_INF_ADDR
    addi.!c1    r_scratch, r0, ASIC_SEM_RNMPR_FREE_INF_ADDR
    CAPRI_NEXT_TABLE_READ_e(1, TABLE_LOCK_DIS,
                            proxyc_s7_page2_free,
                            r_scratch,
                            TABLE_SIZE_64_BITS)
    nop

    .align

/*
 * Entered after having acquired a page free index semaphore,
 * this function invokes a common function to free page 1.
 */    
proxyc_s6_page1_free:

    CAPRI_CLEAR_TABLE1_VALID

    /*
     * page free semaphore should never be full
     */
    sne         c1, d.pindex_full, r0
    j.c1        proxyc_page_free_sem_pindex_full
    add         r_page_is_small, r0, k.t1_s2s_aol_A1_small  // delay slot
    add         r_page_addr, r0, k.{t1_s2s_aol_A1_sbit0_ebit3...\
                                    t1_s2s_aol_A1_sbit4_ebit51}
    jal         r_return, proxyc_page_free
    add         r_table_idx, r0, d.pindex                   // delay slot
    b           _page2_free_applic_check
    nop


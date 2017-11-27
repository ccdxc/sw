#include "app_redir_common.h"

struct phv_                         p;
struct rawc_page2_free_k            k;
struct rawc_page2_free_page2_free_d d;

/*
 * Registers usage; must match same definitions in rawc_page0_free.s
 */
#define r_page_addr                 r1  // page address to free
#define r_page_is_small             r2  // small page indicator
#define r_table_base                r3  // RNMPR_TABLE_BASE or RNMPR_SMALL_TABLE_BASE
#define r_table_idx                 r4  // PI index
#define r_return                    r5  // return address
#define r_scratch                   r6
 
%%
    .param      rawc_page_free
    .param      rawc_page_free_sem_pindex_full
    
    .align

/*
 * Entered after having acquired a page free index semaphore,
 * this function invokes a common function to free page 2.
 */    
rawc_s7_page2_free:

    CAPRI_CLEAR_TABLE1_VALID

    /*
     * page free semaphore should never be full
     */
    sne         c1, d.pindex_full, r0
    j.c1        rawc_page_free_sem_pindex_full
    add         r_page_is_small, r0, k.t1_s2s_aol_A2_small  // delay slot
    add         r_page_addr, r0, k.{t1_s2s_aol_A2_sbit0_ebit47...\
                                    t1_s2s_aol_A2_sbit48_ebit51}
    jal         r_return, rawc_page_free
    add         r_table_idx, r0, d.pindex                   // delay slot
    nop.e
    nop

    


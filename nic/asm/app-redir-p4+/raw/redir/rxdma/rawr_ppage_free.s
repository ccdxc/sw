#include "app_redir_common.h"

struct phv_                         p;
struct rawr_ppage_free_k            k;
struct rawr_ppage_free_ppage_free_d d;

/*
 * Registers usage; must match same definitions in rawr_mpage_free.s
 */
#define r_page_addr                 r1  // page address to free
#define r_page_is_small             r2  // small page indicator
#define r_table_base                r3  // RNMPR_TABLE_BASE or RNMPR_SMALL_TABLE_BASE
#define r_table_idx                 r4  // PI index
#define r_return                    r5  // return address
#define r_scratch                   r6

%%
    .param      rawr_page_free_sem_pindex_full
    .param      rawr_page_free
    
    .align

rawr_s7_ppage_free:

    CAPRI_CLEAR_TABLE1_VALID

    /*
     * ppage free semaphore should never be full
     */
    sne         c1, d.pindex_full, r0
    j.c1        rawr_page_free_sem_pindex_full
    addi        r_page_is_small, r0, FALSE   // delay slot
    add         r_page_addr, r0, k.{to_s7_ppage_sbit0_ebit5...\
                                    to_s7_ppage_sbit30_ebit33}
    jal         r_return, rawr_page_free
    add         r_table_idx, r0, d.pindex                   // delay slot
    nop.e
    nop

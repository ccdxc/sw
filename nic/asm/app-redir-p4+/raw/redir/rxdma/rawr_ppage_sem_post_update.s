#include "app_redir_common.h"

struct phv_                                     p;
struct s1_tbl_ppage_pindex_post_update_d        d;

/*
 * Registers usage
 *
 * Nnote that CAPRI_NEXT_TABLE_READ_INDEX uses r1/r2 as scratch registers!
 */
#define r_pi                        r3  // semaphore producer index
#define r_table_base                r4  // semaphore table base

%%
    .param      rawr_ppage_post_alloc
    .param      RAWR_RNMDPR_TABLE_BASE
    .align

/*
 * Page (for storing packet) pindex fetched and updated via HW semaphore,
 * the page itself will now be allocated.
 */
rawr_ppage_sem_pindex_post_update:

    //CAPRI_CLEAR_TABLE0_VALID

    /*
     * If semaphore full, handle it in a later stage but
     * launch a ppage fetch now anyway (at pindex 0)
     */    
    add         r_pi, r0, d.{page_pindex}.wx
    mincr       r_pi, RAWR_RNMDPR_RING_SHIFT, r0
    sne         c1, d.page_pindex_full, r0
    add.c1      r_pi, r0, r0
    phvwri.c1   p.rawr_kivec0_do_cleanup_discard, TRUE

    APP_REDIR_IMM64_LOAD(r_table_base, RAWR_RNMDPR_TABLE_BASE)
    CAPRI_NEXT_TABLE_READ_INDEX(0, r_pi,
                                TABLE_LOCK_DIS,
                                rawr_ppage_post_alloc,
                                r_table_base, 
                                RAWR_RNMDPR_TABLE_ENTRY_SIZE_SHIFT,
                                TABLE_SIZE_64_BITS)
    phvwr.e     p.rawr_kivec0_ppage_sem_pindex_full, d.page_pindex_full
    RAWR_METRICS_VAL_SET(pkt_alloc_errors, d.page_pindex_full) // delay slot
    

#include "app_redir_common.h"

struct phv_                                         p;
struct rawr_ppage_sem_k                             k;
struct rawr_ppage_sem_ppage_pindex_post_update_d    d;

/*
 * Registers usage
 */
#define r_pi                        r1  // semaphore producer index
#define r_table_base                r2  // semaphore table base

%%
    .param      rawr_s2_ppage_post_alloc
    .param      RNMPR_TABLE_BASE
    .align

/*
 * Page (for storing packet) pindex fetched and updated via HW semaphore,
 * the page itself will now be allocated.
 */
rawr_s1_ppage_sem_pindex_post_update:

    CAPRI_CLEAR_TABLE1_VALID
    
    phvwr       p.common_phv_ppage_sem_pindex_full, d.pindex_full
    
    /*
     * If semaphore full, handle it in a later stage but
     * launch a ppage fetch now anyway (at pindex 0)
     */    
    add         r_pi, r0, d.{pindex}.wx
    mincr       r_pi, CAPRI_RNMPR_RING_SHIFT, r0
    sne         c1, d.pindex_full, r0
    add.c1      r_pi, r0, r0

    APP_REDIR_IMM64_LOAD(r_table_base, RNMPR_TABLE_BASE)
    CAPRI_NEXT_TABLE_READ_INDEX(1, r_pi,
                                TABLE_LOCK_DIS,
                                rawr_s2_ppage_post_alloc,
                                r_table_base, 
                                RNMPR_TABLE_ENTRY_SIZE_SHFT,
                                TABLE_SIZE_64_BITS)
    nop.e
    nop   

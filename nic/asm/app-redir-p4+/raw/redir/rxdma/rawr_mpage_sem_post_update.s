#include "app_redir_common.h"

struct phv_                                         p;
struct rawr_mpage_sem_k                             k;
struct rawr_mpage_sem_mpage_pindex_post_update_d    d;

/*
 * Registers usage
 *
 * Note that CAPRI_NEXT_TABLE_READ_INDEX uses r1/r2 as scratch registers!
 */
#define r_pi                        r3  // semaphore producer index
#define r_table_base                r4  // semaphore table base

%%
    .param      rawr_s2_mpage_post_alloc
    .param      RNMPR_SMALL_TABLE_BASE
    .align

/*
 * Page (for storing metaheader) pindex fetched and updated via HW semaphore,
 * the page itself will now be allocated.
 */
rawr_s1_mpage_sem_pindex_post_update:

    CAPRI_CLEAR_TABLE2_VALID
    
    phvwr       p.common_phv_mpage_sem_pindex_full, d.pindex_full
    
    /*
     * If semaphore full, handle it in a later stage but
     * launch an mpage fetch now anyway (at pindex 0)
     */    
    add         r_pi, r0, d.{pindex}.wx
    mincr       r_pi, CAPRI_RNMPR_SMALL_RING_SHIFT, r0
    sne         c1, d.pindex_full, r0
    add.c1      r_pi, r0, r0

    APP_REDIR_IMM64_LOAD(r_table_base, RNMPR_SMALL_TABLE_BASE)
    CAPRI_NEXT_TABLE_READ_INDEX(2, r_pi,
                                TABLE_LOCK_DIS,
                                rawr_s2_mpage_post_alloc,
                                r_table_base, 
                                RNMPR_SMALL_TABLE_ENTRY_SIZE_SHFT,
                                TABLE_SIZE_64_BITS)
    nop.e
    nop   

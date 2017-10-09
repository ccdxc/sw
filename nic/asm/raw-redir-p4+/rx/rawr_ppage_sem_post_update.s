#include "rawr-defines.h"

struct phv_                                         p;
struct rawr_ppage_sem_k                             k;
struct rawr_ppage_sem_ppage_pindex_post_update_d    d;

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
    add         r3, r0, d.{pindex}.wx
    mincr       r3, CAPRI_RNMPR_RING_SHIFT, r0
    seq         c1, d.pindex_full, r0
    bcf         [c1], ppage_fetch_launch
    nop
    add         r3, r0, r0

ppage_fetch_launch:

    RAWR_IMM64_LOAD(r4, RNMPR_TABLE_BASE)
    CAPRI_NEXT_TABLE_READ_INDEX(1, r3,
                                TABLE_LOCK_DIS,
                                rawr_s2_ppage_post_alloc,
	                        r4, 
                                RNMPR_TABLE_ENTRY_SIZE_SHFT,
                                TABLE_SIZE_64_BITS)
    nop.e
    nop   

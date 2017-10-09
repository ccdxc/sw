#include "rawr-defines.h"

struct phv_                                         p;
struct rawr_mpage_sem_k                             k;
struct rawr_mpage_sem_mpage_pindex_post_update_d    d;

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
    add         r3, r0, d.{pindex}.wx
    mincr       r3, CAPRI_RNMPR_SMALL_RING_SHIFT, r0
    seq         c1, d.pindex_full, r0
    bcf         [c1], mpage_fetch_launch
    nop
    add         r3, r0, r0
    

mpage_fetch_launch:

    RAWR_IMM64_LOAD(r4, RNMPR_SMALL_TABLE_BASE)
    CAPRI_NEXT_TABLE_READ_INDEX(2, r3,
                                TABLE_LOCK_DIS,
                                rawr_s2_mpage_post_alloc,
	                        r4, 
                                RNMPR_SMALL_TABLE_ENTRY_SIZE_SHFT,
                                TABLE_SIZE_64_BITS)
    nop.e
    nop   

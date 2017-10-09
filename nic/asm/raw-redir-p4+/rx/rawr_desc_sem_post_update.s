#include "rawr-defines.h"

struct phv_                                     p;
struct rawr_desc_sem_k                          k;
struct rawr_desc_sem_desc_pindex_post_update_d  d;

%%
    .param      rawr_s2_desc_post_alloc
    .param      RNMDR_TABLE_BASE
    .align

/*
 * Descriptor pindex fetched and updated via HW semaphore,
 * the descriptor itself will now be allocated.
 */
rawr_s1_desc_sem_pindex_post_update:

    CAPRI_CLEAR_TABLE0_VALID
	
    phvwr       p.common_phv_desc_sem_pindex_full, d.pindex_full
    
    /*
     * If semaphore full, handle it in a later stage but
     * launch a desc fetch now anyway (at pindex 0)
     */    
    add         r3, r0, d.{pindex}.wx
    mincr       r3, CAPRI_RNMDR_RING_SHIFT, r0
    seq         c1, d.pindex_full, r0
    bcf         [c1], desc_fetch_launch
    nop
    add         r3, r0, r0
    
desc_fetch_launch:

    RAWR_IMM64_LOAD(r4, RNMDR_TABLE_BASE)
    CAPRI_NEXT_TABLE_READ_INDEX(0, r3,
                                TABLE_LOCK_DIS,
                                rawr_s2_desc_post_alloc,
	                        r4, 
                                RNMDR_TABLE_ENTRY_SIZE_SHFT,
                                TABLE_SIZE_64_BITS)
    nop.e
    nop


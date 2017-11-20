#include "app_redir_common.h"

struct phv_                                         p;
struct proxyr_mpage_sem_k                           k;
struct proxyr_mpage_sem_mpage_pindex_post_update_d  d;

%%
    .param      proxyr_s4_mpage_post_alloc
    .param      RNMPR_SMALL_TABLE_BASE
    .align

/*
 * Page (for storing metaheader) pindex fetched and updated via HW semaphore,
 * the page itself will now be allocated.
 */
proxyr_s3_mpage_sem_pindex_post_update:

    CAPRI_CLEAR_TABLE0_VALID
    
    phvwr       p.common_phv_mpage_sem_pindex_full, d.pindex_full
    
    /*
     * If semaphore full, handle it in a later stage but
     * launch an mpage fetch now anyway (at pindex 0)
     */    
    add         r3, r0, d.{pindex}.wx
    mincr       r3, CAPRI_RNMPR_SMALL_RING_SHIFT, r0
    sne         c1, d.pindex_full, r0
    add.c1      r3, r0, r0

    APP_REDIR_IMM64_LOAD(r4, RNMPR_SMALL_TABLE_BASE)
    CAPRI_NEXT_TABLE_READ_INDEX(0, r3,
                                TABLE_LOCK_DIS,
                                proxyr_s4_mpage_post_alloc,
                                r4, 
                                RNMPR_SMALL_TABLE_ENTRY_SIZE_SHFT,
                                TABLE_SIZE_64_BITS)
    nop.e
    nop   

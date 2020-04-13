#include "app_redir_common.h"

struct phv_                                         p;
struct proxyr_mpage_sem_k                           k;
struct proxyr_mpage_sem_pindex_post_update_mpage_d  d;

/*
 * Registers usage
 *
 * Note that CAPRI_NEXT_TABLE_READ_INDEX uses r1/r2 as scratch registers!
 */
#define r_pi                        r3  // semaphore producer index
#define r_table_base                r4  // semaphore table base


%%
    .param      proxyr_s4_mpage_post_alloc
    .param      proxyr_s4_mpage_skip_alloc
    .param      RNMPR_SMALL_TABLE_BASE
    
    .align

/*
 * Page (for storing metaheader) pindex fetched and updated via HW semaphore,
 * the page itself will now be allocated.
 */
proxyr_s3_mpage_sem_pindex_post_update:

    //CAPRI_CLEAR_TABLE0_VALID

    /*
     * If semaphore full, handle it in a later stage but
     * launch an mpage fetch now anyway (at pindex 0)
     */    
    add         r_pi, r0, d.{pindex}.wx
    mincr       r_pi, ASIC_RNMPR_SMALL_RING_SHIFT, r0
    sne         c1, d.pindex_full, r0
    add.c1      r_pi, r0, r0
    phvwri.c1   p.common_phv_do_cleanup_discard, TRUE

    APP_REDIR_IMM64_LOAD(r_table_base, RNMPR_SMALL_TABLE_BASE)
    CAPRI_NEXT_TABLE_READ_INDEX(0, r_pi,
                                TABLE_LOCK_DIS,
                                proxyr_s4_mpage_post_alloc,
                                r_table_base, 
                                RNMPR_SMALL_TABLE_ENTRY_SIZE_SHFT,
                                TABLE_SIZE_64_BITS)
    phvwr.e     p.common_phv_mpage_sem_pindex_full, d.pindex_full
    phvwr       p.t3_s2s_inc_stat_sem_alloc_full, d.pindex_full // delay slot

    .align
    
/*
 * Entered as a stage transition which skipped the meta page
 * semaphore acquisition (when an error condition was detected)
 */    
proxyr_s3_mpage_sem_pindex_skip:

    //CAPRI_CLEAR_TABLE0_VALID
    
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, proxyr_s4_mpage_skip_alloc)
    nop.e
    nop


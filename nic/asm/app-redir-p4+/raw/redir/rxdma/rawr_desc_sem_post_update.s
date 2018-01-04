#include "app_redir_common.h"

struct phv_                                     p;
struct rawr_desc_sem_k                          k;
struct rawr_desc_sem_desc_pindex_post_update_d  d;

/*
 * Registers usage
 *
 * Note that CAPRI_NEXT_TABLE_READ_INDEX uses r1/r2 as scratch registers!
 */
#define r_pi                        r3  // semaphore producer index
#define r_table_base                r4  // semaphore table base

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
    phvwr       p.t3_s2s_inc_stat_desc_sem_alloc_full, d.pindex_full
    
    /*
     * If semaphore full, handle it in a later stage but
     * launch a desc fetch now anyway (at pindex 0)
     */    
    add         r_pi, r0, d.{pindex}.wx
    mincr       r_pi, CAPRI_RNMDR_RING_SHIFT, r0
    sne         c1, d.pindex_full, r0
    add.c1      r_pi, r0, r0

    APP_REDIR_IMM64_LOAD(r_table_base, RNMDR_TABLE_BASE)
    CAPRI_NEXT_TABLE_READ_INDEX(0, r_pi,
                                TABLE_LOCK_DIS,
                                rawr_s2_desc_post_alloc,
                                r_table_base, 
                                RNMDR_TABLE_ENTRY_SIZE_SHFT,
                                TABLE_SIZE_64_BITS)
    nop.e
    nop


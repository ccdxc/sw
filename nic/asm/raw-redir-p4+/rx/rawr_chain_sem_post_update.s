#include "rawr-defines.h"

struct phv_                                             p;
struct rawr_chain_sem_k                                 k;
struct rawr_chain_sem_chain_sem_pindex_post_update_d    d;

%%
    .param      rawr_s6_cleanup_discard
    .param      rawr_s6_chain_xfer
    
    .align

/*
 * Next service chain pindex fetched and updated via HW semaphore.
 */
rawr_s6_chain_sem_pindex_post_update:

    CAPRI_CLEAR_TABLE1_VALID
	
    /*
     * If next service chain semaphore full, clean up
     */    
    sne         c1, d.pindex_full, r0
    RAWR_IMM64_LOAD(r4, rawr_s6_cleanup_discard)
    jrcf        [c1], r4

    /*
     * Pass the obtained pindex to a common DMA transfer function via r1
     */
    add         r1, r0, d.{pindex}.wx
    mincr       r1, k.common_phv_chain_rxq_ring_size_shift, r0
    j           rawr_s6_chain_xfer
    nop


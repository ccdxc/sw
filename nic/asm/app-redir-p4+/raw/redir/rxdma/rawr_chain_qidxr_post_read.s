#include "app_redir_common.h"

struct phv_                                             p;
struct rawr_chain_qidxr_k                               k;
struct rawr_chain_qidxr_chain_qidxr_pindex_post_read_d  d;

/*
 * Registers usage
 */
#define r_chain_pindex              r1  // must match rawr_chain_xfer.s
#define r_ring_select               r2  // ring index select

 
%%
    .param      rawr_s6_chain_xfer
    
    .align

/*
 * Next service chain queue index table post read handling.
 * Upon entry in this stage, the table has been locked
 * allowing for atomic read-update.
 */
rawr_s6_chain_qidxr_pindex_post_read:

    CAPRI_CLEAR_TABLE2_VALID
        
    /*
     * TODO: check for queue full
     */    
    /*
     * Evaluate which per-core queue applies
     */
    add         r_ring_select, r0, k.common_phv_chain_ring_index_select
    seq         c1, r_ring_select, CHAIN_QIDXR_PI_0
    seq         c2, r_ring_select, CHAIN_QIDXR_PI_1
    seq         c3, r_ring_select, CHAIN_QIDXR_PI_2

    /*
     * Pass the obtained pindex to a common DMA transfer function via r1
     */
    add         r_chain_pindex, r0, d.pi_0
    tbladd.c1   d.pi_0, 1
    add.c2      r_chain_pindex, r0, d.pi_1
    tbladd.c2   d.pi_1, 1
    add.c3      r_chain_pindex, r0, d.pi_2
    tbladd.c3   d.pi_2, 1
    
    j           rawr_s6_chain_xfer
    mincr       r_chain_pindex, k.{common_phv_chain_ring_size_shift_sbit0_ebit1...\
                                   common_phv_chain_ring_size_shift_sbit2_ebit4}, r0 // delay slot


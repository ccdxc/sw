#include "raw_redir_common.h"

struct phv_                                                 p;
struct rawr_chain_txq_pindex_k                              k;
struct rawr_chain_txq_pindex_chain_txq_pindex_post_read_d   d;

%%
    .param      rawr_s6_cleanup_discard
    .param      rawr_s6_chain_xfer
    
    .align

/*
 * Next service chain TxQ ring pindex/cindex post read handling.
 * We check for queue full and launch cleanup if so.
 * Otherwise, branch to common code to set up DMA transfers.
 */
rawr_s6_chain_txq_pindex_post_read:

    CAPRI_CLEAR_TABLE1_VALID
        
    /*
     * If next service chain TxQ ring full, launch clean up.
     * Ring is full when PI+1 == CI
     */    
    add         r1, r0, d.{pi_curr}.hx
    add         r2, r0, d.{ci_curr}.hx
    mincr       r1, k.{common_phv_chain_ring_size_shift_sbit0_ebit1...\
                       common_phv_chain_ring_size_shift_sbit2_ebit4}, 1
    mincr       r2, k.{common_phv_chain_ring_size_shift_sbit0_ebit1...\
                       common_phv_chain_ring_size_shift_sbit2_ebit4}, r0
    seq         c1, r1, r2
    RAWR_IMM64_LOAD(r4, rawr_s6_cleanup_discard)
    jrcf        [c1], r4

    /*
     * Pass the obtained pindex to a common DMA transfer function via r1
     */
    subi        r1, r1, 1                                                // delay slot
    j           rawr_s6_chain_xfer
    mincr       r1, k.{common_phv_chain_ring_size_shift_sbit0_ebit1...\
                       common_phv_chain_ring_size_shift_sbit2_ebit4}, r0 // delay slot


#include "app_redir_common.h"

struct phv_                             p;
struct rawr_chain_pindex_pre_alloc_k    k;

/*
 * Registers usage
 */
#define r_ring_indices_addr         r1

%%

    .param      rawr_s6_chain_txq_pindex_post_read
    .param      rawr_s6_chain_qidxr_pindex_post_read
    .param      rawr_s6_cleanup_discard
    .align
    
rawr_s5_chain_pindex_pre_alloc:

    CAPRI_CLEAR_TABLE0_VALID

    /*
     * For a given flow, one of 2 types of redirection applies:
     *   1) Redirect to ARM CPU RxQ, or
     *   2) Redirect to a P4+ TxQ
     *
     * Proceed only if none of the desc/ppage/mpage semaphores 
     */
    sne         c1, k.common_phv_desc_sem_pindex_full, r0
    sne         c2, k.common_phv_ppage_sem_pindex_full, r0
    sne         c3, k.common_phv_mpage_sem_pindex_full, r0
    bcf         [c1 | c2 | c3], _cleanup_discard_launch

    /*
     * Based on redirection type, launch the corresponding read
     * of ring indices.
     */
    seq         c1, k.common_phv_chain_to_rxq, r0   // delay slot
    bcf         [c1], _chain_txq_ring_indices_launch
    add         r_ring_indices_addr, r0, k.{to_s5_chain_ring_indices_addr}.wx    // delay slot

    /*
     * Chain to ARM RxQ: access HBM queue index table directly
     */
    CAPRI_NEXT_TABLE_READ(2, TABLE_LOCK_EN,
                          rawr_s6_chain_qidxr_pindex_post_read,
                          r_ring_indices_addr,
                          TABLE_SIZE_256_BITS)
    nop.e
    nop


/*
 * Launch read of chain TxQ's current PI/CI, each index is 16 bits wide.
 * For the current flow, the assumption is we are the only producer
 * for the corresponding TxQ ring.
 */
_chain_txq_ring_indices_launch:
     
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS,
                          rawr_s6_chain_txq_pindex_post_read,
                          r_ring_indices_addr,
                          TABLE_SIZE_32_BITS)
    nop.e
    nop                          
    

/*
 * Launch common cleanup code for next stage
 */
_cleanup_discard_launch:    

    /*
     * TODO: add stats here
     */
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, rawr_s6_cleanup_discard)
    nop.e
    nop

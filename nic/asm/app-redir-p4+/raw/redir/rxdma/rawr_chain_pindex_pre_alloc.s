#include "app_redir_common.h"

struct phv_                             p;
struct rawr_chain_pindex_pre_alloc_k    k;

/*
 * Registers usage
 *
 * Note that CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP uses r1/r2 as scratch registers!
 */
#define r_ring_indices_addr         r3
#define r_qstate_addr               r4

%%

    .param      rawr_s4_chain_txq_pindex_post_read
    .param      rawr_s4_chain_sem_pindex_post_update
    .param      rawr_s4_cleanup_discard
    .param      rawr_err_stats_inc
    
    .align
    
rawr_s3_chain_pindex_pre_alloc:

    //CAPRI_CLEAR_TABLE1_VALID

    /*
     * For a given flow, one of 2 types of redirection applies:
     *   1) Redirect to ARM CPU RxQ, or
     *   2) Redirect to a P4+ TxQ
     *
     * Proceed only if cleanup_discard had not been signaled
     */
    sne         c1, k.common_phv_do_cleanup_discard, r0
    bcf         [c1], _cleanup_discard_launch

    /*
     * Based on redirection type, launch the corresponding read
     * of ring indices.
     */
    seq         c1, k.common_phv_chain_to_rxq, r0   // delay slot
    bcf         [c1], _chain_txq_ring_indices_launch
    add         r_ring_indices_addr, r0, \
                k.{to_s3_chain_ring_indices_addr_sbit0_ebit31...\
                   to_s3_chain_ring_indices_addr_sbit32_ebit33} // delay slot
    CAPRI_NEXT_TABLE_READ_e(1, 
                            TABLE_LOCK_DIS,
                            rawr_s4_chain_sem_pindex_post_update,
                            r_ring_indices_addr,
                            TABLE_SIZE_64_BITS)
    nop


/*
 * Launch read of chain TxQ's current PI/CI, each index is 16 bits wide.
 * For the current flow, the assumption is we are the only producer
 * for the corresponding TxQ ring.
 */
_chain_txq_ring_indices_launch:
     
    CAPRI_NEXT_TABLE_READ_e(1, TABLE_LOCK_DIS,
                            rawr_s4_chain_txq_pindex_post_read,
                            r_ring_indices_addr,
                            TABLE_SIZE_32_BITS)
    nop                          
    

_cleanup_discard_launch:    

    CAPRI_CLEAR_TABLE1_VALID
    
    /*
     * Launch common cleanup code for next stage
     */
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, rawr_s4_cleanup_discard)
    nop.e
    nop

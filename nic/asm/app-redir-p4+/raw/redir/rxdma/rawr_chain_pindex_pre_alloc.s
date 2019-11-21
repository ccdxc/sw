#include "app_redir_common.h"

struct phv_             p;
struct s4_tbl_k         k;

/*
 * Registers usage
 *
 * Note that CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP uses r1/r2 as scratch registers!
 */
#define r_ring_indices_addr         r4

%%

    .param      rawr_chain_txq_pindex_post_read
    .param      rawr_chain_sem_pindex_post_update
    .param      rawr_cleanup_discard
    
    .align
    
rawr_chain_pindex_pre_alloc:

    //CAPRI_CLEAR_TABLE0_VALID
    
    /*
     * For a given flow, one of 2 types of redirection applies:
     *   1) Redirect to ARM CPU RxQ, or
     *   2) Redirect to a P4+ TxQ
     *
     * Proceed only if cleanup_discard had not been signaled
     */
    bbeq        RAWR_KIVEC0_DO_CLEANUP_DISCARD, 1, _cleanup_discard_launch
    nop
    
    /*
     * Based on redirection type, launch the corresponding read
     * of ring indices.
     */
    bbeq        RAWR_KIVEC0_CHAIN_TO_RXQ, 0, _chain_txq_ring_indices_launch
    add         r_ring_indices_addr, r0, RAWR_KIVEC1_CHAIN_RING_INDICES_ADDR    // delay slot
    
    CPU_ARQ_SEM_INF_ADDR(RAWR_KIVEC0_CHAIN_RING_INDEX_SELECT,
                         r_ring_indices_addr)
    CAPRI_NEXT_TABLE_READ_e(0, 
                            TABLE_LOCK_DIS,
                            rawr_chain_sem_pindex_post_update,
                            r_ring_indices_addr,
                            TABLE_SIZE_64_BITS)
    nop


/*
 * Launch read of chain TxQ's current PI/CI, each index is 16 bits wide.
 * For the current flow, the assumption is we are the only producer
 * for the corresponding TxQ ring.
 */
_chain_txq_ring_indices_launch:
     
    CAPRI_NEXT_TABLE_READ_e(0, TABLE_LOCK_DIS,
                            rawr_chain_txq_pindex_post_read,
                            r_ring_indices_addr,
                            TABLE_SIZE_32_BITS)
    nop                          
    

_cleanup_discard_launch:    

    /*
     * Launch common cleanup code for next stage
     */
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP_e(0, rawr_cleanup_discard)

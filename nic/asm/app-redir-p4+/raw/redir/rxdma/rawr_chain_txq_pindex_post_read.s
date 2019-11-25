#include "app_redir_common.h"

struct phv_                     p;
struct s5_tbl1_k                k;
struct s5_tbl1_txq_post_read_d  d;

/*
 * Registers usage
 */
#define r_chain_pindex              r1  // must match rawr_chain_xfer.s
#define r_pi                        r2
#define r_ci                        r3


%%
    .param      rawr_cleanup_discard
    .param      rawr_chain_xfer
    
    .align

/*
 * Next service chain TxQ ring pindex/cindex post read handling.
 * We check for queue full and launch cleanup if so.
 * Otherwise, branch to common code to set up DMA transfers.
 */
rawr_chain_txq_pindex_post_read:

    /*
     * Table1 valid will be cleared by rawr_chain_xfer or rawr_cleanup_discard
     */
        
    /*
     * If next service chain TxQ ring full, launch clean up.
     * Ring is full when PI+1 == CI
     */    
    add         r_pi, r0, d.{pi_curr}.hx
    add         r_ci, r0, d.{ci_curr}.hx
    mincr       r_pi, RAWR_KIVEC0_CHAIN_RING_SIZE_SHIFT, 1
    mincr       r_ci, RAWR_KIVEC0_CHAIN_RING_SIZE_SHIFT, r0
    beq         r_pi, r_ci, _txq_ring_full_discard
    add         r_chain_pindex, r0, d.{pi_curr}.hx      // delay slot
    
    /*
     * Pass the obtained pindex to a common DMA transfer function via r1
     */
    j           rawr_chain_xfer
    nop


/*
 * TxQ ring full discard
 */                       
_txq_ring_full_discard:

    j           rawr_cleanup_discard
    RAWR_METRICS_SET(txq_full_discards)                 // delay slot

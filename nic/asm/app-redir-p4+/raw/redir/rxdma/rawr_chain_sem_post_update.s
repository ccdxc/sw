#include "app_redir_common.h"
#include "../../../../cpu-p4plus/include/cpu-macros.h"

struct phv_                     p;
struct s5_tbl1_k                k;
struct s5_tbl1_post_update_d    d;

/*
 * Registers usage
 */
#define r_chain_pindex              r1  // must match rawr_chain_xfer.s

%%
    .param      rawr_cleanup_discard
    .param      rawr_chain_xfer
    
    .align

/*
 * Next service chain queue pindex fetched and updated via HW semaphore.
 */
rawr_chain_sem_pindex_post_update:

    /*
     * Table1 valid will be cleared by rawr_chain_xfer or rawr_cleanup_discard
     */
    
    sne         c1, d.arq_full, r0
    j.c1        rawr_cleanup_discard
    RAWR_METRICS_SET_c(c1, rxq_full_discards)           // delay slot
    
    /*
     * Pass the obtained pindex to a common DMA transfer function via r1
     */
    j           rawr_chain_xfer
    add         r_chain_pindex, r0, d.{arq_pindex}.wx   // delay slot


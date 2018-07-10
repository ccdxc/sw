#include "app_redir_common.h"
#include "../../../../cpu-p4plus/include/cpu-macros.h"

struct phv_                 p;
struct rawr_chain_sem_k     k;
struct rawr_chain_sem_d     d;

/*
 * Registers usage
 */
#define r_chain_pindex              r1  // must match rawr_chain_xfer.s

 
%%
    .param      rawr_s5_chain_xfer
    
    .align

/*
 * Next service chain queue pindex fetched and updated via HW semaphore.
 */
rawr_s5_chain_sem_pindex_post_update:

    CAPRI_CLEAR_TABLE1_VALID
        
    /*
     * TODO: check for semaphore full when ARQ semaphore has support for it
     */    
    j           rawr_s5_chain_xfer
    add         r_chain_pindex, r0, d.{u.post_update_d.arq_pindex}.wx // delay slot


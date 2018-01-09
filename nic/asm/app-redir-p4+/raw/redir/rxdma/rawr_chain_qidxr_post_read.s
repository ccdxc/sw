#include "app_redir_common.h"
#include "../../../../cpu-p4plus/include/cpu-macros.h"

struct phv_                 p;
struct rawr_chain_pindex_k  k;
struct rawr_chain_pindex_d  d;

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

    CAPRI_CLEAR_TABLE1_VALID
        
    /*
     * TODO: check for queue full
     */    
    /*
     * Evaluate which per-core queue applies
     */
#ifdef DO_NOT_USE_CPU_SEM
    add         r_ring_select, r0, k.common_phv_chain_ring_index_select
    CPU_ARQ_PIDX_READ_INC(r_chain_pindex, r_ring_select, d, pi_0, r3, r4)
#else
    add         r_chain_pindex, r0, d.{u.qidxr_post_read_d.arq_pindex}.wx
#endif    
    j           rawr_s6_chain_xfer
    mincr       r_chain_pindex, k.common_phv_chain_ring_size_shift, r0 // delay slot


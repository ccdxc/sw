#include "app_redir_common.h"

struct phv_                             p;
struct s2_tbl_k                         k;
struct s2_tbl_ppage_post_alloc_d        d;

%%
    .param      rawr_toeplitz_hash_none
    .align

rawr_ppage_post_alloc:

    //CAPRI_CLEAR_TABLE0_VALID
    
    /*
     * Move to common stage to launch chain queue index read.
     */
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, rawr_toeplitz_hash_none)
    
#if !APP_REDIR_VISIBILITY_USE_MIRROR_SESSION

    /*
     * In the case of SPAN, post hashing result will handle the stage advance
     * to rawr_chain_pindex_pre_alloc (see rawr_toeplitz_hash_calc_id) so we 
     * can cancel the stage launch above.
     */
    sne         c1, RAWR_KIVEC0_REDIR_SPAN_INSTANCE, r0
    CAPRI_CLEAR_TABLE0_VALID_COND(c1)
#endif

    seq.e       c1, RAWR_KIVEC0_PPAGE_SEM_PINDEX_FULL, r0
    phvwr.c1    p.rawr_kivec1_ppage, d.page     // delay slot
    

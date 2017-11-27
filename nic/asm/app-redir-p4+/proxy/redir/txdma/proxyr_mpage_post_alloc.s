#include "app_redir_common.h"

struct phv_                                 p;
struct proxyr_mpage_post_k                  k;
struct proxyr_mpage_post_mpage_post_alloc_d d;

/*
 * Registers usage
 */
#define r_qstate_addr               r1


%%

    .param      proxyr_s5_chain_pindex_pre_alloc
    .align

proxyr_s4_mpage_post_alloc:

    CAPRI_CLEAR_TABLE0_VALID

    phvwr       p.to_s6_mpage, d.page
    
    /*
     * Advance to next stage which is stage 5 to eventually arrive
     * at a pre-agreed upon stage for handling chain pindex atomic update.
     */
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, proxyr_s5_chain_pindex_pre_alloc)
    nop.e
    nop

    .align
    
/*
 * Entered as a stage transition which skipped the meta page allocation
 * (when an error condition was detected)
 */    
proxyr_s4_mpage_skip_alloc:

    CAPRI_CLEAR_TABLE1_VALID
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, proxyr_s5_chain_pindex_pre_alloc)
    nop.e
    nop


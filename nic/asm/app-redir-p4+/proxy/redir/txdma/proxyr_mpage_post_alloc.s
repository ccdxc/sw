#include "app_redir_common.h"

struct phv_                             p;
struct proxyr_mpage_k                   k;
struct proxyr_mpage_post_alloc_mpage_d  d;

/*
 * Registers usage
 *
 * Note that CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP uses r1/r2 as scratch registers!
 */
#define r_ring_indices_addr         r3

%%

    .param      proxyr_s5_chain_xfer
    .param      proxyr_s5_cleanup_discard

    .align

proxyr_s4_mpage_post_alloc:

    //CAPRI_CLEAR_TABLE0_VALID

    seq         c1, k.common_phv_mpage_sem_pindex_full, r0
    phvwr.c1    p.to_s5_mpage, d.page
    sne         c1, k.common_phv_do_cleanup_discard, r0
    bcf         [c1], _cleanup_discard_launch

    /*
     * Launch read of chain RxQ indices
     */
    add         r_ring_indices_addr, r0, \
                k.{to_s4_chain_ring_indices_addr_sbit0_ebit31...\
                   to_s4_chain_ring_indices_addr_sbit32_ebit33} // delay slot
    seq         c1, r_ring_indices_addr, r0
    bcf         [c1], _cleanup_discard_launch
    phvwri.c1   p.t3_s2s_inc_stat_null_ring_indices_addr, 1 // delay slot
    
    CAPRI_NEXT_TABLE_READ_e(0, 
                            TABLE_LOCK_DIS,
                            proxyr_s5_chain_xfer,
                            r_ring_indices_addr,
                            TABLE_SIZE_64_BITS)
    nop

    .align
    
/*
 * Entered as a stage transition which skipped the meta page allocation
 * (when an error condition was detected)
 */    
proxyr_s4_mpage_skip_alloc:

    /*
     * Fall through!!!
     */

     
/*
 * Initiate cleanup discard
 */
_cleanup_discard_launch:    

    CAPRI_CLEAR_TABLE0_VALID

    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(1, proxyr_s5_cleanup_discard)
    nop.e
    nop

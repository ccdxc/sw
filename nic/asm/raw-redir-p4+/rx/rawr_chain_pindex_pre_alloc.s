#include "rawr-defines.h"

struct phv_                             p;
struct rawr_chain_pindex_pre_alloc_k    k;

%%

    .param      rawr_s6_chain_sem_pindex_post_update
    .param      rawr_s6_chain_qidxr_pindex_post_read
    .param      rawr_s6_cleanup_discard
    .align
    
rawr_s5_chain_pindex_pre_alloc:

    CAPRI_CLEAR_TABLE0_VALID

    /*
     * Fetch/update next service chain semaphore pindex only if
     * none of the desc/ppage/mpage semaphores were full
     */
    sne         c1, k.common_phv_desc_sem_pindex_full, r0
    sne         c2, k.common_phv_ppage_sem_pindex_full, r0
    sne         c3, k.common_phv_mpage_sem_pindex_full, r0
    bcf         [c1 | c2 | c3], cleanup_discard_launch

    /*
     * First, test to see if chain semaphore is applicable
     */
    add         r3, r0, k.to_s5_chain_rxq_sem_alloc_idx // delay slot
    beq         r3, r0, chain_qidxr_pindex_read
    nop
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, 
                          rawr_s6_chain_sem_pindex_post_update,
                          r3,
                          TABLE_SIZE_64_BITS)
    nop.e
    nop
    
chain_qidxr_pindex_read:

    /*
     * Semaphore not available, access HBM queue index table directly
     */
    add         r3, r0, k.{to_s5_chain_rxq_qidxr_base}.wx
    beq         r3, r0, cleanup_discard_launch
    nop
    CAPRI_NEXT_TABLE_READ(2, TABLE_LOCK_EN,
                          rawr_s6_chain_qidxr_pindex_post_read,
                          r3,
                          TABLE_SIZE_512_BITS)
    nop.e
    nop
     
cleanup_discard_launch:    

    /*
     * Launch common cleanup code for next stage
     */
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, rawr_s6_cleanup_discard)
    nop.e
    nop

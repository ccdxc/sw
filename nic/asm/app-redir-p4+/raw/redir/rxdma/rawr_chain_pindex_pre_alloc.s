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

    .param      rawr_s6_chain_txq_pindex_post_read
    .param      rawr_s6_chain_qidxr_pindex_post_read
    .param      rawr_s6_cleanup_discard
    .param      rawr_err_stats_inc
    
    .align
    
rawr_s5_chain_pindex_pre_alloc:

    CAPRI_CLEAR_TABLE0_VALID

    /*
     * For a given flow, one of 2 types of redirection applies:
     *   1) Redirect to ARM CPU RxQ, or
     *   2) Redirect to a P4+ TxQ
     *
     * Proceed only if none of the desc/ppage/mpage semaphores are full
     */
    sne         c1, k.common_phv_desc_sem_pindex_full, r0
    sne         c2, k.common_phv_ppage_sem_pindex_full, r0
    sne         c3, k.common_phv_mpage_sem_pindex_full, r0
    bcf         [c1 | c2 | c3], _sem_full_discard

    /*
     * Based on redirection type, launch the corresponding read
     * of ring indices.
     */
    seq         c1, k.common_phv_chain_to_rxq, r0   // delay slot
    bcf         [c1], _chain_txq_ring_indices_launch
#ifdef DO_NOT_USE_CPU_SEM
    add         r_ring_indices_addr, r0, k.{to_s5_chain_ring_indices_addr_sbit0_ebit31...\
                                            to_s5_chain_ring_indices_addr_sbit32_ebit33} // delay slot

    /*
     * Chain to ARM RxQ: access HBM queue index table directly
     */
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_EN,
                          rawr_s6_chain_qidxr_pindex_post_read,
                          r_ring_indices_addr,
                          TABLE_SIZE_512_BITS)
#else
    CPU_ARQ_SEM_IDX_INC_ADDR(RX, 
                             k.common_phv_chain_ring_index_select,
                             r_ring_indices_addr)
    CAPRI_NEXT_TABLE_READ(2, 
                          TABLE_LOCK_DIS,
                          rawr_s6_chain_qidxr_pindex_post_read,
                          r_ring_indices_addr,
                          TABLE_SIZE_64_BITS)
#endif
    nop.e
    nop


/*
 * Launch read of chain TxQ's current PI/CI, each index is 16 bits wide.
 * For the current flow, the assumption is we are the only producer
 * for the corresponding TxQ ring.
 */
_chain_txq_ring_indices_launch:
     
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS,
                          rawr_s6_chain_txq_pindex_post_read,
                          r_ring_indices_addr,
                          TABLE_SIZE_32_BITS)
    nop.e
    nop                          
    

/*
 * Discard packet due to semaphore full
 */
_sem_full_discard:    

    RAWRCB_ERR_STAT_INC_LAUNCH(3, r_qstate_addr,
                               k.{common_phv_qstate_addr_sbit0_ebit0... \
                                  common_phv_qstate_addr_sbit33_ebit33},
                               RAWRCB_STAT_SEM_ALLOC_FULL_BYTE_OFFS)
    /*
     * Launch common cleanup code for next stage
     */
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, rawr_s6_cleanup_discard)
    nop.e
    nop

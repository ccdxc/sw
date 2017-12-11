#include "app_redir_common.h"

struct phv_                         p;
struct rawc_my_txq_entry_k          k;
struct rawc_my_txq_entry_consume_d  d;

/*
 * Registers usage
 */
#define r_ci                        r1
#define r_db_addr_scratch           r2
#define r_db_data_scratch           r3
#define r_desc                      r4
#define r_chain_indices_addr        r5

%%

    .param      rawc_s2_pkt_txdma_prep
    .param      rawc_s2_chain_txq_desc_enqueue
    .align
    
/*
 * Descriptor at my TxQ CI has been fetched, we now take one of 2 paths:
 *    1) Prep DMA descriptors for injecting packet data to P4, or
 *    2) Enqueue the descriptor to the next service chain's TXQ if 
 *       configured to do so.
 */
rawc_s1_my_txq_entry_consume:

    CAPRI_CLEAR_TABLE0_VALID
    
    /*
     * Advance CI via doorbell to clear scheduler bit;
     * doorbell address includes: pid_chk (FALSE), lif, qtype.
     * Doorbell data include: pid, qid, ring, CI.
     */
    add         r_ci, r0, k.to_s1_my_txq_ci_curr
    mincr       r_ci, k.to_s1_my_txq_ring_size_shift, 1

    DOORBELL_WRITE_CINDEX(k.{to_s1_my_txq_lif_sbit0_ebit7...\
                             to_s1_my_txq_lif_sbit8_ebit10},
                          k.to_s1_my_txq_qtype,
                          k.to_s1_my_txq_qid,
                          RAWC_MY_TXQ_RING_DEFAULT,
                          r_ci,
                          r_db_addr_scratch,
                          r_db_data_scratch)
    /*
     * Advance past the descriptor scratch area and launch descriptor AOLs read.
     * This is always done before checking for rawccb_ready so desc/pages
     * can be properly freed as needed.
     */
    add         r_desc, r0, d.desc
    phvwr       p.t0_s2s_desc, r_desc
    add         r_desc, r_desc.dx, NIC_DESC_ENTRY_0_OFFSET
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS,
                          rawc_s2_pkt_txdma_prep,
                          r_desc,
                          TABLE_SIZE_512_BITS)
    /*
     * If the next service chain is a TxQ, prepare to enqueue the descriptor
     * to that queue. Otherwise, the pages contained in the descriptor should be 
     * DMA'ed to P4.
     */
    sne         c1, k.common_phv_chain_txq_base, r0
    phvwri.c1   p.common_phv_next_service_chain_action, TRUE
    nop.!c1.e

    /*
     * Take the cleanup discard path (via rawc_s2_pkt_txdma_prep above)
     * if applicable.
     */    
    sne         c1, k.common_phv_do_cleanup_discard, r0   // delay slot
    nop.c1.e
    
    /*
     * Launch read of chain TxQ's current PI/CI, each index is 16 bits wide.
     * For the current flow, the assumption is we are the only producer
     * for the corresponding TxQ ring.
     */
    add         r_chain_indices_addr, r0, \
                k.to_s1_chain_txq_ring_indices_addr     // delay slot
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                          rawc_s2_chain_txq_desc_enqueue,
                          r_chain_indices_addr,
                          TABLE_SIZE_32_BITS)
    nop.e
    nop                          

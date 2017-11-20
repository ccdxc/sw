#include "app_redir_common.h"

struct phv_                         p;
struct rawc_my_txq_entry_k          k;
struct rawc_my_txq_entry_consume_d  d;

#define CI_R                        r1
#define DB_ADDR_SCRATCH_R           r2
#define DB_DATA_SCRATCH_R           r3
#define DESC_R                      r4
#define CHAIN_INDICES_ADDR_R        r5

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
    add         CI_R, r0, k.to_s1_my_txq_ci_curr
    mincr       CI_R, k.to_s1_my_txq_ring_size_shift, 1

    DOORBELL_WRITE_CINDEX(k.{to_s1_my_txq_lif_sbit0_ebit7...\
                             to_s1_my_txq_lif_sbit8_ebit10},
                          k.to_s1_my_txq_qtype,
                          k.to_s1_my_txq_qid,
                          RAWC_MY_TXQ_RING_DEFAULT,
                          CI_R,
                          DB_ADDR_SCRATCH_R,
                          DB_DATA_SCRATCH_R)
    /*
     * Advance past the descriptor scratch area and launch descriptor AOLs read.
     */
    phvwr       p.to_s2_my_txq_desc, d.desc
    add         DESC_R, r0, d.{desc}.dx
    addi        DESC_R, DESC_R, NIC_DESC_ENTRY_0_OFFSET
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                          rawc_s2_pkt_txdma_prep,
                          DESC_R,
                          TABLE_SIZE_512_BITS)
    /*
     * If the next service chain is a TxQ, prepare to enqueue the
     * descriptor to that queue. Otherwise, the packet buffers
     * contained in the descriptor should be DMA'ed to P4.
     */
    sne         c1, k.common_phv_chain_txq_base, r0
    phvwri.c1   p.common_phv_next_service_chain_action, TRUE
    nop.!c1.e
    
    /*
     * Launch read of chain TxQ's current PI/CI, each index is 16 bits wide.
     * For the current flow, the assumption is we are the only producer
     * for the corresponding TxQ ring.
     */
    add         CHAIN_INDICES_ADDR_R, r0, k.{to_s1_chain_txq_ring_indices_addr}.wx
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS,
                          rawc_s2_chain_txq_desc_enqueue,
                          CHAIN_INDICES_ADDR_R,
                          TABLE_SIZE_32_BITS)
    nop.e
    nop                          

#include "app_redir_common.h"

struct phv_                     p;
struct s1_tbl_k                 k;
struct s1_tbl_consume_d         d;

/*
 * Registers usage
 */
#define r_desc                      r1

%%

    .param      rawc_pkt_txdma_prep
    .param      rawc_chain_txq_desc_enqueue
    .align
    
/*
 * Descriptor at my TxQ CI has been fetched, we now take one of 2 paths:
 *    1) Prep DMA descriptors for injecting packet data to P4, or
 *    2) Enqueue the descriptor to the next service chain's TXQ if 
 *       configured to do so.
 */
rawc_my_txq_entry_consume:

    //CAPRI_CLEAR_TABLE0_VALID

    /*
     * Packet descriptor queued from ARM is in host format which is
     * little endian.
     */
    add         r_desc, d.{desc}.dx, NIC_DESC_ENTRY_0_OFFSET
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                          rawc_pkt_txdma_prep,
                          r_desc,
                          TABLE_SIZE_512_BITS)
    /*
     * If the next service chain is a TxQ, prepare to enqueue the descriptor
     * to that queue. Otherwise, the pages contained in the descriptor should be 
     * DMA'ed to P4.
     */
    seq         c1, RAWC_KIVEC0_NEXT_SERVICE_CHAIN_ACTION, r0
    nop.c1.e
    phvwr       p.rawc_kivec3_desc, d.{desc}.dx // delay slot

    /*
     * Launch read of chain TxQ's current PI/CI, each index is 16 bits wide.
     * For the current flow, the assumption is we are the only producer
     * for the corresponding TxQ ring.
     */
    CAPRI_NEXT_TABLE_READ_e(1, TABLE_LOCK_DIS,
                            rawc_chain_txq_desc_enqueue,
                            RAWC_KIVEC1_CHAIN_TXQ_RING_INDICES_ADDR,
                            TABLE_SIZE_32_BITS)
    nop                          

#include "app_redir_common.h"

struct phv_                             p;
struct proxyr_my_txq_entry_k            k;
struct proxyr_my_txq_entry_consume_d    d;

/*
 * Registers usage
 */
#define r_ci                        r1
#define r_db_addr_scratch           r2
#define r_db_data_scratch           r3
#define r_desc                      r4

%%

    .param      proxyr_s2_desc_post_read
    .align
    
/*
 * Descriptor at my TxQ CI has been fetched, we now take one of 2 paths:
 *    1) Prep DMA descriptors for injecting packet data to P4, or
 *    2) Enqueue the descriptor to the next service chain's TXQ if 
 *       configured to do so.
 */
proxyr_s1_my_txq_entry_consume:

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
                          k.to_s1_my_txq_ring,
                          r_ci,
                          r_db_addr_scratch,
                          r_db_data_scratch)
    /*
     * Advance past the descriptor scratch area and launch descriptor AOLs read.
     */
    add         r_desc, r0, d.desc
    phvwr       p.to_s6_desc, r_desc
    add         r_desc, r_desc.dx, NIC_DESC_ENTRY_0_OFFSET
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                          proxyr_s2_desc_post_read,
                          r_desc,
                          TABLE_SIZE_512_BITS)
    nop.e
    nop                          

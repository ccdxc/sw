#include "app_redir_common.h"

struct phv_                             p;
struct proxyr_my_txq_entry_k            k;
struct proxyr_my_txq_entry_consume_d    d;

#define CI_R                        r1
#define DB_ADDR_SCRATCH_R           r2
#define DB_DATA_SCRATCH_R           r3
#define DESC_R                      r4

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
    add         CI_R, r0, k.to_s1_my_txq_ci_curr
    mincr       CI_R, k.to_s1_my_txq_ring_size_shift, 1

    DOORBELL_WRITE_CINDEX(k.{to_s1_my_txq_lif_sbit0_ebit7...\
                             to_s1_my_txq_lif_sbit8_ebit10},
                          k.to_s1_my_txq_qtype,
                          k.to_s1_my_txq_qid,
                          PROXYR_MY_TXQ_RING_DEFAULT,
                          CI_R,
                          DB_ADDR_SCRATCH_R,
                          DB_DATA_SCRATCH_R)
    /*
     * Advance past the descriptor scratch area and launch descriptor AOLs read.
     */
    phvwr       p.to_s6_desc, d.desc
    phvwri      p.common_phv_desc_valid, TRUE
    add         DESC_R, r0, d.{desc}.dx
    addi        DESC_R, DESC_R, NIC_DESC_ENTRY_0_OFFSET
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                          proxyr_s2_desc_post_read,
                          DESC_R,
                          TABLE_SIZE_512_BITS)
    nop.e
    nop                          

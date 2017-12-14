#include "app_redir_common.h"

struct phv_                             p;
struct proxyc_my_txq_entry_k            k;
struct proxyc_my_txq_entry_consume_d    d;

/*
 * Registers usage
 */
#define r_ci                        r1
#define r_db_addr_scratch           r2
#define r_db_data_scratch           r3
#define r_desc                      r4

%%

    .param      proxyc_s2_desc_meta_strip
    .align
    
/*
 * Descriptor at my TxQ CI has been fetched, we now launch a read
 * for the AOLs so meta headers can be stripped before the descriptor
 * can be enqueued to the next service in the chain.
 */
proxyc_s1_my_txq_entry_consume:

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
                          PROXYC_MY_TXQ_RING_DEFAULT,
                          r_ci,
                          r_db_addr_scratch,
                          r_db_data_scratch)
    /*
     * Advance past the descriptor scratch area and launch descriptor AOLs read.
     * This descriptor was submitted from ARM so it is in host order
     */
    add         r_desc, r0, d.{desc}.dx
    phvwr       p.to_s3_desc, r_desc
    phvwr       p.t0_s2s_desc, r_desc
    add         r_desc, r_desc, NIC_DESC_ENTRY_0_OFFSET
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                          proxyc_s2_desc_meta_strip,
                          r_desc,
                          TABLE_SIZE_512_BITS)
    nop.e
    nop                          

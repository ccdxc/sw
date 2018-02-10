#include "app_redir_common.h"

struct phv_                             p;
struct proxyc_my_txq_entry_k            k;
struct proxyc_my_txq_entry_consume_d    d;

/*
 * Registers usage
 */
#define r_desc                      r1

%%

    .param      proxyc_s2_desc_meta_strip
    .align
    
/*
 * Descriptor at my TxQ CI has been fetched, we now launch a read
 * for the AOLs so meta headers can be stripped before the descriptor
 * can be enqueued to the next service in the chain.
 */
proxyc_s1_my_txq_entry_consume:

    //CAPRI_CLEAR_TABLE0_VALID
    
    /*
     * Advance past the descriptor scratch area and launch descriptor AOLs read.
     * This descriptor was submitted from ARM so it is in host order
     */
    add         r_desc, r0, d.{desc}.dx
    phvwr       p.to_s3_desc, r_desc
    phvwr       p.t0_s2s_desc, r_desc
    add         r_desc, r_desc, NIC_DESC_ENTRY_0_OFFSET
    CAPRI_NEXT_TABLE_READ_e(0, TABLE_LOCK_EN,
                            proxyc_s2_desc_meta_strip,
                            r_desc,
                            TABLE_SIZE_512_BITS)
    nop


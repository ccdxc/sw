#include "app_redir_common.h"

struct phv_                             p;
struct proxyr_my_txq_entry_k            k;
struct proxyr_my_txq_entry_consume_d    d;

/*
 * Registers usage
 */
#define r_desc                      r1

%%

    .param      proxyr_s2_desc_post_read
    .align
    
/*
 * Descriptor at my TxQ CI has been fetched
 */
proxyr_s1_my_txq_entry_consume:

    //CAPRI_CLEAR_TABLE0_VALID
    
    /*
     * Advance past the descriptor scratch area and launch descriptor AOLs read.
     * This descriptor was submitted from TCP/TLS so it is in native NCC order
     */
    add         r_desc, r0, d.desc
    phvwr       p.to_s5_desc, r_desc
    add         r_desc, r_desc, NIC_DESC_ENTRY_0_OFFSET
    CAPRI_NEXT_TABLE_READ_e(0, TABLE_LOCK_DIS,
                            proxyr_s2_desc_post_read,
                            r_desc,
                            TABLE_SIZE_512_BITS)
    nop                          

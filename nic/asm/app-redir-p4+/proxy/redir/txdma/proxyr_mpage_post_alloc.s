#include "app_redir_common.h"

struct phv_                             p;
struct proxyr_mpage_k                   k;
struct proxyr_mpage_mpage_post_alloc_d  d;

%%

    .param      proxyr_s5_flow_key_post_read
    .align


proxyr_s4_mpage_post_alloc:

    CAPRI_CLEAR_TABLE0_VALID

    phvwr       p.to_s6_mpage, d.page
    
    /*
     * Advance to next stage which is stage 5 to eventually arrive
     * at a pre-agreed upon stage for handling chain pindex atomic update,
     * at the same time fetch the flow key info from our qstate.
     */
    add         r3, r0, k.{to_s4_qstate_addr_sbit0_ebit31...\
                           to_s4_qstate_addr_sbit32_ebit33}
    addi        r3, r3, PROXYR_CB_TABLE_FLOW_KEY_OFFSET
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                          proxyr_s5_flow_key_post_read,
                          r3,
                          TABLE_SIZE_512_BITS)
    nop.e
    nop

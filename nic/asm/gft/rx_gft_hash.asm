#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct rx_gft_hash_k k;
struct rx_gft_hash_d d;
struct phv_ p;

%%

rx_gft_hash_info:
    seq.c1      c1, d.rx_gft_hash_info_d.entry_valid, TRUE
    bcf         [!c1], rx_gft_hash_miss
    phvwr.c1    p.flow_action_metadata_flow_index, d.rx_gft_hash_info_d.flow_index
    phvwr       p.flow_action_metadata_policer_index, \
                    d.rx_gft_hash_info_d.policer_index

    // copy flow index (12 bits) to the top bits of ethernet src address
    // for packet verification
    // phvwr       p.ethernet_1_srcAddr[47:36], d.rx_gft_hash_info_d.flow_index

    // copy hint only if is non-zero
    sne         c1, d.rx_gft_hash_info_d.hint9, r0
    phvwr.c1    p.flow_action_metadata_overflow_lkp, TRUE
    or.e        r1, d.rx_gft_hash_info_d.hint9, 1, 31
    phvwr.c1    p.flow_lkp_metadata_overflow_hash, r1

rx_gft_hash_miss:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rx_gft_hash_error:
    nop.e
    nop

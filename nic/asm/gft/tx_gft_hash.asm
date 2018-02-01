#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct tx_gft_hash_k k;
struct tx_gft_hash_d d;
struct phv_ p;

%%

tx_gft_hash_info:
    seq.c1      c1, d.tx_gft_hash_info_d.entry_valid, TRUE
    bcf         [!c1], tx_gft_hash_miss
    phvwr.c1    p.flow_action_metadata_flow_index, d.tx_gft_hash_info_d.flow_index
    phvwr       p.flow_action_metadata_policer_index, \
                    d.tx_gft_hash_info_d.policer_index

    // copy hint only if is non-zero
    sne         c1, d.tx_gft_hash_info_d.hint9, r0
    phvwr.c1    p.flow_action_metadata_overflow_lkp, TRUE
    or.e        r1, d.tx_gft_hash_info_d.hint9, 1, 31
    phvwr.c1    p.flow_lkp_metadata_overflow_hash, r1

tx_gft_hash_miss:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tx_gft_hash_error:
    nop.e
    nop

#include "tyche.h"
#include "egress.h"
#include "EGRESS_p.h"

struct tx_flow_hash_k k;
struct tx_flow_hash_d d;
struct phv_ p;

%%

tx_flow_hash_info:
    seq.c1      c1, d.tx_flow_hash_info_d.entry_valid, TRUE
    bcf         [!c1], tx_flow_hash_miss
    phvwr.c1    p.flow_action_metadata_flow_index, \
                    d.tx_flow_hash_info_d.flow_index
    phvwr       p.flow_action_metadata_prio, d.tx_flow_hash_info_d.prio
    phvwrpair   p.flow_action_metadata_parent_policer_index[12:10], \
                    d.tx_flow_hash_info_d.parent_policer_index_sbit0_ebit2, \
                    p.flow_action_metadata_parent_policer_index[9:0], \
                    d.tx_flow_hash_info_d.parent_policer_index_sbit3_ebit12
    phvwr       p.flow_action_metadata_child_policer_index, \
                    d.tx_flow_hash_info_d.child_policer_index
    phvwr       p.capri_intrinsic_tm_oport, d.tx_flow_hash_info_d.oport

    // copy hint only if is non-zero
    sne         c1, d.tx_flow_hash_info_d.hint9, r0
    phvwr.c1    p.flow_action_metadata_overflow_lkp, TRUE
    or.e        r1, d.tx_flow_hash_info_d.hint9, 1, 31
    phvwr.c1    p.flow_lkp_metadata_overflow_hash, r1

tx_flow_hash_miss:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tx_flow_hash_error:
    nop.e
    nop

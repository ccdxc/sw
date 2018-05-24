#include "tyche.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct rx_flow_result_k k;
struct phv_ p;

%%

rx_flow_result:
    slt         c1, k.flow_action_metadata_tcam_prio, k.flow_action_metadata_prio
    nop.!c1.e
    phvwr.c1    p.flow_action_metadata_flow_index, \
                    k.flow_action_metadata_tcam_flow_index
    phvwr       p.flow_action_metadata_parent_policer_index, \
                    k.flow_action_metadata_tcam_parent_policer_index
    phvwr       p.flow_action_metadata_child_policer_index, \
                    k.flow_action_metadata_tcam_child_policer_index
    phvwr.e     p.capri_intrinsic_lif, k.flow_action_metadata_tcam_lif
    phvwr       p.capri_intrinsic_tm_oport, k.flow_action_metadata_tcam_tm_oport

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rx_flow_result_error:
    nop.e
    nop

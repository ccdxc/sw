#include "tyche.h"
#include "egress.h"
#include "EGRESS_p.h"

struct tx_flow_tcam_k k;
struct tx_flow_tcam_d d;
struct phv_ p;

%%

nop:
    nop.e
    nop

tx_flow_tcam_info:
    phvwr       p.flow_action_metadata_tcam_prio, d.u.tx_flow_tcam_info_d.prio
    phvwr       p.flow_action_metadata_tcam_index, \
                    d.u.tx_flow_tcam_info_d.tcam_index
    phvwr       p.flow_action_metadata_tcam_flow_index, \
                    d.u.tx_flow_tcam_info_d.flow_index
    phvwrpair.e p.flow_action_metadata_tcam_parent_policer_index, \
                    d.u.tx_flow_tcam_info_d.parent_policer_index, \
                    p.flow_action_metadata_tcam_child_policer_index, \
                    d.u.tx_flow_tcam_info_d.child_policer_index
    phvwr       p.flow_action_metadata_tcam_tm_oport, \
                    d.u.tx_flow_tcam_info_d.oport


/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tx_flow_tcam_error:
    nop.e
    nop

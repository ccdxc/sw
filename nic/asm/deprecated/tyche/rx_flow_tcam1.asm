#include "tyche.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct rx_flow_tcam1_k k;
struct rx_flow_tcam1_d d;
struct phv_ p;

%%

nop:
    nop.e
    nop

rx_flow_tcam_info:
    phvwr       p.flow_action_metadata_tcam_prio, d.u.rx_flow_tcam_info_d.prio
    phvwr       p.flow_action_metadata_tcam_index, \
                    d.u.rx_flow_tcam_info_d.tcam_index
    phvwr       p.flow_action_metadata_tcam_flow_index, \
                    d.u.rx_flow_tcam_info_d.flow_index
    phvwrpair   p.flow_action_metadata_tcam_parent_policer_index, \
                    d.u.rx_flow_tcam_info_d.parent_policer_index, \
                    p.flow_action_metadata_tcam_child_policer_index, \
                    d.u.rx_flow_tcam_info_d.child_policer_index
    phvwr.e     p.flow_action_metadata_tcam_lif, d.u.rx_flow_tcam_info_d.lif
    phvwr       p.flow_action_metadata_tcam_tm_oport, \
                    d.u.rx_flow_tcam_info_d.oport


/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rx_flow_tcam_error:
    nop.e
    nop

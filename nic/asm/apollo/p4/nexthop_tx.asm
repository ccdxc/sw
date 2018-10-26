#include "egress.h"
#include "EGRESS_p.h"
#include "apollo.h"

struct nexthop_tx_k k;
struct nexthop_tx_d d;
struct phv_         p;

%%

nexthop_info:
    phvwr       p.rewrite_metadata_tep_index, d.nexthop_info_d.tep_index
    phvwr       p.nat_metadata_snat_required, d.nexthop_info_d.snat_required
    phvwr       p.rewrite_metadata_encap_type, d.nexthop_info_d.encap_type
    phvwr       p.rewrite_metadata_dst_slot_id, d.nexthop_info_d.dst_slot_id
    phvwr.e     p.policer_metadata_traffic_class, d.nexthop_info_d.traffic_class
    phvwr       p.rewrite_metadata_mytep_ip, r5

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
nexthop_tx_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

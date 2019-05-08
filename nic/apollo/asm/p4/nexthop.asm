#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_nexthop_k.h"
#include "apollo.h"

struct nexthop_k_   k;
struct nexthop_d    d;
struct phv_         p;

%%

nexthop_info:
    seq         c1, k.rewrite_metadata_nexthop_index, r0
    bcf         [c1], nexthop_invalid
    phvwr.!c1   p.rewrite_metadata_tep_index, d.nexthop_info_d.tep_index
    phvwr       p.nat_metadata_snat_required, d.nexthop_info_d.snat_required
    phvwr       p.rewrite_metadata_encap_type, d.nexthop_info_d.encap_type
    seq         c1, k.rewrite_metadata_dst_slot_id_valid, 0
    phvwr.c1    p.rewrite_metadata_dst_slot_id, d.nexthop_info_d.dst_slot_id
    phvwr.e     p.policer_metadata_traffic_class, d.nexthop_info_d.traffic_class
    phvwr       p.rewrite_metadata_mytep_ip, r5

nexthop_invalid:
    phvwr.e     p.control_metadata_p4e_drop_reason[P4E_DROP_INVALID_NEXTHOP], 1
    phvwr       p.capri_intrinsic_drop, 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
nexthop_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

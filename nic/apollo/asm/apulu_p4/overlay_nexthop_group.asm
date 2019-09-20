#include "apulu.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_overlay_nexthop_group_k.h"

struct overlay_nexthop_group_k_ k;
struct overlay_nexthop_group_d  d;
struct phv_                     p;

%%

overlay_nexthop_group_info:
    add             r2, d.overlay_nexthop_group_info_d.num_nexthops, 1
    mod             r1, k.p4e_i2e_entropy_hash[3:0], r2
    phvwr           p.rewrite_metadata_nexthop_type, \
                        d.overlay_nexthop_group_info_d.nexthop_type
    seq             c1, d.overlay_nexthop_group_info_d.num_nexthops, r0
    phvwr.c1.e      p.txdma_to_p4e_nexthop_id, \
                        d.overlay_nexthop_group_info_d.nexthop_id
    add.e           r1, r1, d.overlay_nexthop_group_info_d.nexthop_id
    phvwr.f         p.txdma_to_p4e_nexthop_id, r1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
overlay_nexthop_group_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

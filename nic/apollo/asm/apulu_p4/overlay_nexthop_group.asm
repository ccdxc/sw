#include "apulu.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_overlay_nexthop_group_k.h"

struct overlay_nexthop_group_k_ k;
struct overlay_nexthop_group_d  d;
struct phv_                     p;

%%

overlay_nexthop_group_info:
    seq             c1, k.rewrite_metadata_nexthop_group_id, r0
    nop.c1.e
    add             r2, d.overlay_nexthop_group_info_d.num_nexthop_groups, 1
    mod             r1, k.p4e_i2e_entropy_hash[3:0], r2

    seq             c1, d.overlay_nexthop_group_info_d.num_nexthop_groups, r0
    phvwr.c1.e      p.rewrite_metadata_nexthop_group_id, \
                        d.overlay_nexthop_group_info_d.nexthop_group_id
    add.e           r1, r1, d.overlay_nexthop_group_info_d.nexthop_group_id
    phvwr.f         p.rewrite_metadata_nexthop_group_id, r1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
overlay_nexthop_group_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

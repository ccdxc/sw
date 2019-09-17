#include "apulu.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_underlay_nexthop_group_k.h"

struct underlay_nexthop_group_k_ k;
struct underlay_nexthop_group_d  d;
struct phv_                     p;

%%

underlay_nexthop_group_info:
    seq             c1, k.rewrite_metadata_nexthop_group_id, r0
    nop.c1.e
    add             r2, d.underlay_nexthop_group_info_d.num_nexthops, 1
    mod             r1, k.p4e_i2e_entropy_hash[3:0], r2

    seq             c1, d.underlay_nexthop_group_info_d.num_nexthops, r0
    phvwr.c1.e      p.rewrite_metadata_nexthop_id, \
                        d.underlay_nexthop_group_info_d.nexthop_id
    add.e           r1, r1, d.underlay_nexthop_group_info_d.nexthop_id
    phvwr.f         p.rewrite_metadata_nexthop_id, r1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
underlay_nexthop_group_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

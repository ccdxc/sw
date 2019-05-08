#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_nexthop_group_k.h"
#include "apollo.h"

struct nexthop_group_k_ k;
struct nexthop_group_d  d;
struct phv_             p;

%%

nexthop_group_info:
    add             r2, d.nexthop_group_info_d.num_nexthops, 1
    mod             r1, k.p4e_apollo_i2e_entropy_hash[3:0], r2
    seq             c1, d.nexthop_group_info_d.nexthop_index, r0
    phvwr.c1.e      p.rewrite_metadata_nexthop_index, \
                        k.txdma_to_p4e_header_nexthop_group_index
    seq             c1, d.nexthop_group_info_d.num_nexthops, r0
    phvwr.c1.e      p.rewrite_metadata_nexthop_index, \
                        d.nexthop_group_info_d.nexthop_index
    add.e           r1, r1, d.nexthop_group_info_d.nexthop_index
    phvwr           p.rewrite_metadata_nexthop_index, r1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
nexthop_group_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

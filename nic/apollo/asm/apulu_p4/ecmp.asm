#include "apulu.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_ecmp_k.h"

struct ecmp_k_  k;
struct ecmp_d   d;
struct phv_     p;

%%

ecmp_info:
    add             r2, d.ecmp_info_d.num_nexthops, 1
    mod             r1, k.p4e_i2e_entropy_hash[3:0], r2
    phvwr           p.rewrite_metadata_nexthop_type, d.ecmp_info_d.nexthop_type
    seq             c1, d.ecmp_info_d.num_nexthops, r0
    bcf             [c1], ecmp_singleton
    seq             c1, d.ecmp_info_d.nexthop_type, NEXTHOP_TYPE_TUNNEL
    add.!c1         r1, r1, d.ecmp_info_d.nexthop_base
    add.c1          r1, r0, r1, 4
    srl.c1          r1, d.{ecmp_info_d.tunnel_id4...ecmp_info_d.tunnel_id1}, r1
    nop.e
    phvwr.f         p.p4e_i2e_nexthop_id, r1

ecmp_singleton:
    cmov.e          r1, c1, d.ecmp_info_d.tunnel_id1, d.ecmp_info_d.nexthop_base
    phvwr.f         p.p4e_i2e_nexthop_id, r1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
ecmp_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

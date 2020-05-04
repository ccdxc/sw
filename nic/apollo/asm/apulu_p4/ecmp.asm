#include "apulu.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_ecmp_k.h"

struct ecmp_k_  k;
struct ecmp_d   d;
struct phv_     p;

%%

ecmp_info:
    seq             c1, d.ecmp_info_d.num_nexthops, r0
    bcf             [c1], ecmp_invalid
    mod             r1, k.p4e_i2e_entropy_hash[7:0], d.ecmp_info_d.num_nexthops
    phvwr           p.rewrite_metadata_nexthop_type, d.ecmp_info_d.nexthop_type
    seq             c1, d.ecmp_info_d.nexthop_type, NEXTHOP_TYPE_TUNNEL
    bcf             [c1], ecmp_tunnel
    add.!c1.e       r1, d.ecmp_info_d.nexthop_base, r1
    phvwr.f         p.p4e_i2e_nexthop_id, r1

ecmp_tunnel:
    add             r1, r0, r1, 4
    srl.e           r1, d.{ecmp_info_d.tunnel_id4...ecmp_info_d.tunnel_id1}, r1
    phvwr.f         p.p4e_i2e_nexthop_id, r1

ecmp_invalid:
    phvwr.e         p.control_metadata_p4e_drop_reason[P4E_DROP_NEXTHOP_INVALID], 1
    phvwr.f         p.capri_intrinsic_drop, 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
ecmp_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

#include "apulu.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_tunnel_k.h"

struct tunnel_k_    k;
struct tunnel_d     d;
struct phv_         p;

%%

tunnel_info:
    add             r2, d.tunnel_info_d.num_nexthops, 1
    mod             r1, k.p4e_i2e_entropy_hash[3:0], r2
    seq             c1, d.tunnel_info_d.num_nexthops, r0
    phvwr.c1        p.p4e_i2e_nexthop_id, d.tunnel_info_d.nexthop_base
    add             r1, r1, d.tunnel_info_d.nexthop_base
    phvwr.!c1       p.p4e_i2e_nexthop_id, r1
    seq             c1, d.tunnel_info_d.ip_type, IPTYPE_IPV4
    phvwr.c1        p.ipv4_0_dstAddr, d.tunnel_info_d.dipo
    phvwr.!c1       p.ipv6_0_dstAddr, d.tunnel_info_d.dipo
    phvwr.e         p.rewrite_metadata_ip_type, d.tunnel_info_d.ip_type
    phvwr.f         p.rewrite_metadata_tunnel_dmaci, d.tunnel_info_d.dmaci

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tunnel_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

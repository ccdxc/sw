#include "apulu.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_tunnel_k.h"

struct tunnel_k_    k;
struct tunnel_d     d;
struct phv_         p;

%%

tunnel_info:
    seq             c1, d.tunnel_info_d.num_nexthops, r0
    bcf             [c1], tunnel_invalid
    mod             r1, k.p4e_i2e_entropy_hash[3:0], \
                        d.tunnel_info_d.num_nexthops
    seq             c1, d.tunnel_info_d.ip_type, IPTYPE_IPV4
    phvwr.c1        p.ipv4_0_dstAddr, d.tunnel_info_d.dipo
    phvwr.!c1       p.ipv6_0_dstAddr, d.tunnel_info_d.dipo
    phvwr           p.rewrite_metadata_tunnel_vni, d.tunnel_info_d.vni
    phvwr           p.rewrite_metadata_ip_type, d.tunnel_info_d.ip_type
    phvwr           p.rewrite_metadata_tunnel_dmaci, d.tunnel_info_d.dmaci
    phvwr           p.rewrite_metadata_tunnel_tos_override, \
                        d.tunnel_info_d.tos_override
    phvwr           p.rewrite_metadata_tunnel_tos2, d.tunnel_info_d.tos
    add.e           r1, d.tunnel_info_d.nexthop_base, r1
    phvwr.f         p.p4e_i2e_nexthop_id, r1

tunnel_invalid:
    phvwr.e         p.control_metadata_p4e_drop_reason[P4E_DROP_NEXTHOP_INVALID], 1
    phvwr.f         p.capri_intrinsic_drop, 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tunnel_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

#include "apulu.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_vpc_k.h"

struct vpc_k_   k;
struct vpc_d    d;
struct phv_     p;

%%

vpc_info:
    phvwr           p.rewrite_metadata_vni, d.vpc_info_d.vni
    phvwr           p.rewrite_metadata_vrmac, d.vpc_info_d.vrmac

    seq             c1, k.rewrite_metadata_flags[P4_REWRITE_TTL_BITS], \
                        P4_REWRITE_TTL_DEC
    nop.!c1.e
    phvwr           p.rewrite_metadata_tunnel_tos, d.vpc_info_d.tos

    // TTL decrement
    seq             c1, k.ipv4_1_valid, TRUE
    sub             r1, k.ipv4_1_ttl, 1
    phvwr.c1.e      p.control_metadata_update_checksum, 1
    phvwr.c1.f      p.ipv4_1_ttl, r1
    sub.e           r1, k.ipv6_1_hopLimit, 1
    phvwr.f         p.ipv6_1_hopLimit, r1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
vpc_info_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

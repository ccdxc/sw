#include "apulu.h"
#include "egress.h"
#include "EGRESS_p.h"

struct vpc_d    d;
struct phv_     p;

%%

vpc_info:
    phvwr           p.rewrite_metadata_tunnel_tos, d.vpc_info_d.tos
    phvwr.e         p.rewrite_metadata_vni, d.vpc_info_d.vni
    phvwr.f         p.rewrite_metadata_vrmac, d.vpc_info_d.vrmac

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
vpc_info_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

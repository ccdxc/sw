#include "artemis.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_nexthop_k.h"

struct nexthop_k_ k;
struct nexthop_d  d;
struct phv_ p;

%%

nexthop_info:
    phvwr           p.txdma_to_p4e_valid, FALSE
    phvwr           p.predicate_header_valid, FALSE
    phvwr.e         p.p4e_i2e_valid, FALSE
    phvwr           p.capri_intrinsic_tm_oport, TM_PORT_UPLINK_1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
nexthop_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

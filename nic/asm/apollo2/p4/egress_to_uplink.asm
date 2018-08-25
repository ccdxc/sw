#include "apollo.h"
#include "egress.h"
#include "EGRESS_p.h"

struct egress_to_uplink_k k;
struct phv_ p;

%%

egress_to_uplink:
    phvwr       p.capri_intrinsic_tm_oport, TM_PORT_UPLINK_1
    phvwr       p.capri_intrinsic_tm_iq, k.capri_intrinsic_tm_oq
    phvwr       p.capri_txdma_intrinsic_valid, FALSE
    phvwr       p.predicate_header_valid, FALSE
    phvwr.e     p.txdma_to_p4e_header_valid, FALSE
    phvwr       p.p4e_apollo_i2e_valid, FALSE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
egress_to_uplink_error:
    nop.e
    nop

#include "apollo_rxdma.h"
#include "INGRESS_p.h"

struct phv_ p;

%%

udp_start_q:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
udp_q_drain_error:
    phvwr.e         p.capri_intr_drop, 1
    nop

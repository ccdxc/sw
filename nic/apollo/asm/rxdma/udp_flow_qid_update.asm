#include "apollo_rxdma.h"
#include "INGRESS_p.h"

struct phv_ p;

%%

read_update_qid:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
udp_flow_qid_update_error:
    phvwr.e         p.capri_intr_drop, 1
    nop

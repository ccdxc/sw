#include "artemis_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct txdma_enqueue_k  k;
struct txdma_enqueue_d  d;
struct phv_             p;

%%

pkt_enqueue:
    phvwr.e     p.capri_intr_drop, 1
    nop

txdma_q_full:
    phvwr.e     p.capri_intr_drop, 1
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
txdma_enqueue_error:
    phvwr.e     p.capri_intr_drop, 1
    nop

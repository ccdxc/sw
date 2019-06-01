#include "artemis_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct txdma_enqueue_k  k;
struct txdma_enqueue_d  d;
struct phv_             p;

%%

pkt_enqueue:
    /* Is this the last pass? */
    seq          c1, k.capri_p4_intr_recirc_count, 2
    /* Then enque to Tx. Drop for now */
    phvwr.c1     p.capri_intr_drop, 1
    /* Else do nothing */
    nop.e
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

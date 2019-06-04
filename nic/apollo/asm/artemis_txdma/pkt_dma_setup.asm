#include "artemis_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_pkt_dma_setup_k.h"

struct pkt_dma_setup_k_  k;
struct phv_              p;

%%

pkt_dma_setup:
    phvwr           p.capri_txdma_intr_dma_cmd_ptr, \
                        CAPRI_PHV_START_OFFSET(doorbell_ci_update_dma_cmd_pad)/16
    CAPRI_RING_DOORBELL_DATA(0, k.capri_txdma_intr_qid, 0, k.txdma_control_cindex)
    phvwr.e         p.{doorbell_data_pid...doorbell_data_index}, r3.dx
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
pkt_dma_setup_error:
    phvwr.e         p.capri_intr_drop, 1
    nop

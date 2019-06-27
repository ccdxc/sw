#include "artemis_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_iflow_entry_k.h"

struct iflow_entry_k_       k;
struct iflow_entry_d        d;
struct phv_                 p;

%%

iflow:
    add             r1, 0, TXDMA_IFLOW_PARENT_FLIT * 512
    phvwrp          r1, 0, 512, d.iflow_entry_d.flow
    phvwr           p.{txdma_predicate_flow_enable...txdma_predicate_cps_path_en}, 0
    phvwr           p.capri_p4_intr_recirc, FALSE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
iflow_error:
    phvwr.e         p.capri_intr_drop, 1
    nop

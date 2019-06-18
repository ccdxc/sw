#include "artemis_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_iflow_entry_k.h"

struct iflow_entry_k_       k;
struct iflow_entry_d        d;
struct phv_                 p;

%%

iflow:
    phvwr           p.txdma_predicate_cps_path_en, FALSE
    phvwr.e         p.txdma_predicate_flow_enable, FALSE
    phvwr           p.capri_p4_intr_recirc, FALSE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
iflow_error:
    phvwr.e         p.capri_intr_drop, 1
    nop

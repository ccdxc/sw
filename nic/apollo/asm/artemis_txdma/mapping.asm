#include "artemis_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct mapping_k  k;
struct mapping_d  d;
struct phv_       p;

%%

mapping:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
mapping_error:
    phvwr.e     p.capri_intr_drop, 1
    nop

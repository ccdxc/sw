#include "artemis_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct remote_46_mapping_k  k;
struct remote_46_mapping_d  d;
struct phv_       p;

%%

remote_46_mapping:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
remote_46_mapping_error:
    phvwr.e     p.capri_intr_drop, 1
    nop

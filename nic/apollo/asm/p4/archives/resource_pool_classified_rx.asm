#include "apollo.h"
#include "INGRESS_p.h"

struct phv_ p;

%%

resource_pool_classified_info:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
resource_pool_classified_rx_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

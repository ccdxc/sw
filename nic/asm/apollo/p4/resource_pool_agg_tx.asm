#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"

%%

resource_pool_agg_info:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
resource_pool_agg_tx_error:
    nop.e
    nop

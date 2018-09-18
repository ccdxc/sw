#include "apollo.h"

%%

resource_pool_agg_info:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
resource_pool_agg_rx_error:
    nop.e
    nop

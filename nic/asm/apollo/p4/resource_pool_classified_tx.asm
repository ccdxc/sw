#include "apollo.h"

%%

resource_pool_classified_info:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
resource_pool_classified_tx_error:
    nop.e
    nop

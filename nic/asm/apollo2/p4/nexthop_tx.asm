#include "apollo.h"

%%

nexthop_info:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
nexthop_tx_error:
    nop.e
    nop

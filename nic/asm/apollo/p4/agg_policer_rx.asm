#include "apollo.h"

%%

execute_agg_policer:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
agg_policer_rx_error:
    nop.e
    nop

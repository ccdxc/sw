#include "apollo.h"

%%

agg_policer_stats:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
agg_policer_rx_stats_error:
    nop.e
    nop

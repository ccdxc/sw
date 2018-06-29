#include "apollo.h"

%%

policy_stats:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
policy_stats_error:
    nop.e
    nop

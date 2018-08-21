#include "apollo.h"

%%

p4i_drop_stats:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
p4i_drop_stats_error:
    nop.e
    nop

#include "apollo.h"

%%

lpm_s0:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
lpm_s0_error:
    nop.e
    nop

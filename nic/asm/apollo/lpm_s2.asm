#include "apollo.h"

%%

lpm_s2:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
lpm_s2_error:
    nop.e
    nop

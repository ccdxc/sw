#include "apollo.h"

%%

nop:
    nop.e
    nop

rewrite:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rewrite_error:
    nop.e
    nop

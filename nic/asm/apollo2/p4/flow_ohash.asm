#include "flow.asm"

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
flow_ohash_error:
    nop.e
    nop

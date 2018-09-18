#include "source_guard.asm"

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
source_guard_ohash_error:
    nop.e
    nop

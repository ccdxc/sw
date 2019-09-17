#include "local_mapping.asm"

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
local_mapping_ohash_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

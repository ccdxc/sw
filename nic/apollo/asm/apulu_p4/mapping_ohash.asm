#include "mapping.asm"

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
mapping_ohash_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

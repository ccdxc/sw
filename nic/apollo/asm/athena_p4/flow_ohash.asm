#include "flow.asm"

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
flow_ohash_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    phvwr           p.capri_p4_intrinsic_valid, TRUE

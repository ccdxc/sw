#include "ipv4_flow.asm"

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
ipv4_flow_ohash_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

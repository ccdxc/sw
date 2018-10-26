#include "udp_flow_hash3.asm"

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
udp_flow_overflow3_error:
    phvwr.e         p.capri_intr_drop, 1
    nop

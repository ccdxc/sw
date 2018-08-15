#include "udp_flow_hash2.asm"

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
udp_flow_overflow2_error:
    nop.e
    nop

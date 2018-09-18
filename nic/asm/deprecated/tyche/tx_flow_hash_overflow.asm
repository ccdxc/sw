#include "tx_flow_hash.asm"

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tx_flow_hash_overflow_error:
    nop.e
    nop

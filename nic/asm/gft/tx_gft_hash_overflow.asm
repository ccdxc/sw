#include "tx_gft_hash.asm"

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tx_gft_hash_overflow_error:
    nop.e
    nop

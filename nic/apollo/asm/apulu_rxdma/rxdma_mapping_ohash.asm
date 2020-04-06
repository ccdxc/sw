#include "rxdma_mapping.asm"

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rxdma_mapping_ohash_error:
    phvwr.e         p.capri_intr_drop, 1
    nop

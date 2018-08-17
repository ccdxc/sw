#include "apollo_txdma.h"

%%

lpm_s1:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
lpm_s1_error:
    nop.e
    nop

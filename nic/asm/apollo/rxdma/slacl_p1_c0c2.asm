#include "apollo_rxdma.h"

%%

slacl_p1_c0c2:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_p1_c0c2_error:
    nop.e
    nop

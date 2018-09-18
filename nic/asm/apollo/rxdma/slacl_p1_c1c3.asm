#include "apollo_rxdma.h"

%%

slacl_p1_c1c3:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_p1_c1c3_error:
    nop.e
    nop

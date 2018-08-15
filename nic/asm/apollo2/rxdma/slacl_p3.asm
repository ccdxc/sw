#include "apollo_rxdma.h"

%%

slacl_action:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_p3_error:
    nop.e
    nop

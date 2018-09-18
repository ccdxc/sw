#include "apollo_rxdma.h"

%%

slacl_p2_c6c4:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_p2_c6c4_error:
    nop.e
    nop

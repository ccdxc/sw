#include "apollo_rxdma.h"

%%

slacl_dport:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_dport_error:
    nop.e
    nop

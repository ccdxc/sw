#include "apollo_rxdma.h"

%%

udp_flow_info:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
udp_flow_hash0_error:
    nop.e
    nop

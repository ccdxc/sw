#include "apollo_rxdma.h"

%%

udp_start_q:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
udp_q_drain_error:
    nop.e
    nop

#include "apollo_rxdma.h"

%%

read_update_qid:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
udp_flow_qid_update_virtual_error:
    nop.e
    nop

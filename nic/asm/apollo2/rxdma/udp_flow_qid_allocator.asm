#include "apollo_rxdma.h"

%%

qid_alloc_free:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
udp_flow_qid_allocator_error:
    nop.e
    nop

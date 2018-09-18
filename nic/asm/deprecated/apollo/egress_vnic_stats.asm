#include "apollo.h"

%%

egress_vnic_stats:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
egress_vnic_stats_error:
    nop.e
    nop

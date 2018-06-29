#include "apollo.h"

%%

ingress_vnic_stats:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
ingress_vnic_stats_error:
    nop.e
    nop

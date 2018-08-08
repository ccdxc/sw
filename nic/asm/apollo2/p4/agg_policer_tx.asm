#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"

%%

execute_agg_policer:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
agg_policer_tx_error:
    nop.e
    nop

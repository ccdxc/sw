#include "apollo.h"

%%

execute_classified_policer:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
classified_policer_tx_error:
    nop.e
    nop

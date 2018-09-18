#include "apollo.h"

%%

source_guard_info:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
source_guard_error:
    nop.e
    nop

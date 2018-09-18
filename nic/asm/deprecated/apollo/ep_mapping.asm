#include "apollo.h"

%%

ep_mapping_info:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
ep_mapping_error:
    nop.e
    nop

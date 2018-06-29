#include "apollo.h"

%%

slacl_ip_15_00:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_ip_15_00_error:
    nop.e
    nop

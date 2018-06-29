#include "apollo.h"

%%

slacl_ip_31_16:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_ip_31_16_error:
    nop.e
    nop

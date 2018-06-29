#include "apollo.h"

%%

slacl_class0:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_class0_error:
    nop.e
    nop

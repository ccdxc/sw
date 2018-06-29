#include "apollo.h"

%%

slacl_class1:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_class1_error:
    nop.e
    nop

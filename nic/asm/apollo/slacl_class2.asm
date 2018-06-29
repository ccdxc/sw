#include "apollo.h"

%%

slacl_class2:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_class2_error:
    nop.e
    nop

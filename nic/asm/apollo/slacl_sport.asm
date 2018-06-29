#include "apollo.h"

%%

slacl_sport:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_sport_error:
    nop.e
    nop

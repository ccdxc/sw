#include "apollo.h"

%%

encap_mapping:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
encap_error:
    nop.e
    nop

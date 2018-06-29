#include "apollo.h"

%%

slacl_proto:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_proto_error:
    nop.e
    nop

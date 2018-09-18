#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"

%%

nacl_permit:
    nop.e
    nop

.align
nacl_drop:

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
nacl_error:
    nop.e
    nop

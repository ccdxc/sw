#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct phv_ p;

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
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

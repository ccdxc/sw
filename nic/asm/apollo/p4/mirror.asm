#include "apollo.h"
#include "EGRESS_p.h"

struct phv_ p;

%%

nop:
    nop.e
    nop

.align
erspan:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
mirror_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

#include "artemis.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_nat_k.h"

struct nat_k_ k;
struct nat_d  d;
struct phv_ p;

%%

nat_rewrite:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
nat_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

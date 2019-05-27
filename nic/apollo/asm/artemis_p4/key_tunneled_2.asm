#include "artemis.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_key_tunneled_2_k.h"

struct key_tunneled_2_k_ k;
struct key_tunneled_2_d  d;
struct phv_ p;

%%

nop:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
key_tunneled_2_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

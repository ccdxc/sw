#include "artemis.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_flow_k.h"

struct flow_k_ k;
struct flow_d  d;
struct phv_ p;

%%

flow_hash:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
flow_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

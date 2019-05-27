#include "artemis.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_p4i_drop_stats_k.h"

struct p4i_drop_stats_k_ k;
struct p4i_drop_stats_d  d;
struct phv_ p;

%%

p4i_drop_stats:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
p4i_drop_stats_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

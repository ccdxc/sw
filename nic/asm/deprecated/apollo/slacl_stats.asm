#include "apollo.h"
#include "egress.h"
#include "EGRESS_p.h"

struct slacl_stats_k k;
struct slacl_stats_d d;
struct phv_ p;

%%

slacl_stats:
    phvwr.e         p.capri_intrinsic_tm_oport, 0
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_stats_error:
    nop.e
    nop

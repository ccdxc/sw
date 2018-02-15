#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct tx_ipolicer_d d;
struct phv_ p;

%%

tx_policer:
    seq             c1, d.tx_policer_d.entry_valid, TRUE
    seq.c1          c1, d.tx_policer_d.tbkt[39], TRUE
    nop.e
    phvwr.c1.f      p.capri_intrinsic_drop, TRUE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tx_ipolicer_error:
    nop.e
    nop

#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct rx_ipolicer_d d;
struct phv_ p;

%%

rx_policer:
    seq             c1, d.rx_policer_d.entry_valid, TRUE
    seq.c1          c1, d.rx_policer_d.tbkt[39], TRUE
    nop.e
    phvwr.c1        p.capri_intrinsic_drop, TRUE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rx_ipolicer_error:
    nop.e
    nop

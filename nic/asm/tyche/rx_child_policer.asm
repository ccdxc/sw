#include "tyche.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct rx_child_policer_d d;
struct phv_ p;

%%

rx_child_policer:
    seq             c1, d.execute_policer_d.entry_valid, TRUE
    seq.c1          c1, d.execute_policer_d.tbkt[39], TRUE
    nop.e
    phvwr.c1.f      p.capri_intrinsic_drop, TRUE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rx_child_policer_error:
    nop.e
    nop

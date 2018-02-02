#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct tx_vport_k k;
struct tx_vport_d d;
struct phv_ p;

%%

tx_vport:
    phvwr       p.capri_intrinsic_tm_iq, k.capri_intrinsic_tm_oq
    phvwr.e     p.capri_intrinsic_tm_oport, d.tx_vport_d.port
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tx_vport_error:
    nop.e
    nop

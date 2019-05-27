#include "artemis.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_tep2_rx_k.h"

struct tep2_rx_k_ k;
struct tep2_rx_d  d;
struct phv_ p;

%%

tep2_rx_info:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tep2_rx_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

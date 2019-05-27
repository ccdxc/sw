#include "artemis.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_tep1_rx_k.h"

struct tep1_rx_k_ k;
struct tep1_rx_d  d;
struct phv_ p;

%%

tep1_rx_info:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tep1_rx_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

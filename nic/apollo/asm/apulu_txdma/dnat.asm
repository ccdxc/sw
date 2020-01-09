#include "nic/apollo/asm/include/apulu.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_dnat_k.h"

struct dnat_k_        k;
struct phv_           p;
struct dnat_d         d;

%%

dnat:
    phvwr.e   p.txdma_control_lpm1_key[127:64], d.dnat_d.dnat_address[127:64]
    phvwr.f   p.txdma_control_lpm1_key[63:0], d.dnat_d.dnat_address[63:0]


/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
dnat_error:
    phvwr.e         p.capri_intr_drop, 1
    nop

#include "artemis_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct nat_rxdma_k  k;
struct nat_rxdma_d  d;
struct phv_       p;

%%

nat_rxdma:
    sne c2, d.find_local_ip_d.nat_ip, 0
    phvwr.c2 p.p4_to_rxdma_flow_dst, d.find_local_ip_d.nat_ip
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
nat_rxdma_error:
    phvwr.e     p.capri_intr_drop, 1
    nop

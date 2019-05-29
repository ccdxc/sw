#include "artemis_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct vnic_info_k  k;
struct vnic_info_d  d;
struct phv_             p;

%%

vnic_info:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
vnic_info_error:
    phvwr.e     p.capri_intr_drop, 1
    nop

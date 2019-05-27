#include "artemis.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_vnic_mapping_k.h"

struct vnic_mapping_k_ k;
struct vnic_mapping_d  d;
struct phv_ p;

%%

vnic_mapping_info:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
vnic_mapping_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

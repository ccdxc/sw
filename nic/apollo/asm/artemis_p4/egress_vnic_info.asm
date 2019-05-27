#include "artemis.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_egress_vnic_info_k.h"

struct egress_vnic_info_k_ k;
struct egress_vnic_info_d  d;
struct phv_ p;

%%

egress_vnic_info:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
egress_vnic_info_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

#include "artemis.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_vnic_rx_stats_k.h"

struct vnic_rx_stats_k_ k;
struct vnic_rx_stats_d  d;
struct phv_ p;

%%

vnic_rx_stats:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
vnic_rx_stats_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

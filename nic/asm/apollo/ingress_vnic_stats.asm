#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct ingress_vnic_stats_k k;
struct ingress_vnic_stats_d d;
struct phv_ p;

%%

ingress_vnic_stats:
    phvwr.e         p.capri_intrinsic_tm_oport, TM_PORT_EGRESS
    phvwr           p.capri_p4_intrinsic_valid, TRUE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
ingress_vnic_stats_error:
    nop.e
    nop

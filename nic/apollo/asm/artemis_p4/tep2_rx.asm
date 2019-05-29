#include "artemis.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_tep2_rx_k.h"

struct tep2_rx_k_ k;
struct tep2_rx_d  d;
struct phv_ p;

%%

tep2_rx_info:
    phvwr.!c1.e     p.control_metadata_p4i_drop_reason[P4I_DROP_TEP2_RX_MISS], 1
    phvwr.!c1       p.capri_intrinsic_drop, TRUE
    phvwr.e.f       p.vnic_metadata_src_vpc_id, d.tep2_rx_info_d.src_vpc_id
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tep2_rx_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

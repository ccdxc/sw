#include "artemis.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_tep1_rx_k.h"

struct tep1_rx_k_ k;
struct tep1_rx_d  d;
struct phv_ p;

%%

tep1_rx_info:
    phvwr.!c1.e     p.control_metadata_p4i_drop_reason[P4I_DROP_TEP1_RX_MISS], 1
    phvwr.!c1       p.capri_intrinsic_drop, TRUE
    phvwr.e         p.tunnel_metadata_decap_next, d.tep1_rx_info_d.decap_next
    phvwr.f         p.vnic_metadata_src_vpc_id, d.tep1_rx_info_d.src_vpc_id

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tep1_rx_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

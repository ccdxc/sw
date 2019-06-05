#include "artemis.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_egress_vnic_info_k.h"

struct egress_vnic_info_k_ k;
struct egress_vnic_info_d  d;
struct phv_ p;

%%

egress_vnic_info:
    phvwr           p.rewrite_metadata_pa_mac, d.egress_vnic_info_d.pa_mac
    seq             c1, k.control_metadata_direction, RX_FROM_SWITCH
    nop.!c1.e
    seq             c1, k.rewrite_metadata_flags[RX_REWRITE_SMAC_BITS], \
                        RX_REWRITE_SMAC_FROM_VRMAC
    phvwr.e         p.ethernet_1_dstAddr, d.egress_vnic_info_d.ca_mac
    phvwr.c1        p.ethernet_1_srcAddr, d.egress_vnic_info_d.vr_mac

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
egress_vnic_info_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

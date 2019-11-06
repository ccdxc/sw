#include "apulu.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_lif2_k.h"

struct lif2_k_  k;
struct lif2_d   d;
struct phv_     p;

%%

lif_info:
    phvwr           p.capri_intrinsic_lif, k.arm_to_p4i_lif
    phvwr           p.control_metadata_rx_packet, d.lif_info_d.lif_type
    phvwr           p.p4i_i2e_rx_packet, d.lif_info_d.lif_type
    phvwr           p.vnic_metadata_vnic_id, d.lif_info_d.vnic_id
    phvwr           p.vnic_metadata_bd_id, d.lif_info_d.bd_id
    phvwr           p.vnic_metadata_vpc_id, d.lif_info_d.vpc_id
    phvwr           p.control_metadata_pinned_lif, d.lif_info_d.pinned_lif
    phvwr.e         p.control_metadata_pinned_qtype, d.lif_info_d.pinned_qtype
    phvwr.f         p.control_metadata_pinned_qid, d.lif_info_d.pinned_qid

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
lif_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

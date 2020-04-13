#include "apulu.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_vnic_k.h"

struct vnic_k_  k;
struct vnic_d   d;
struct phv_     p;

%%

vnic_info:
    phvwr           p.p4i_to_arm_epoch, d.vnic_info_d.epoch
    seq             c1, k.control_metadata_flow_done, TRUE
    seq.c1          c1, k.control_metadata_flow_miss, FALSE
    //sne.c1          c1, k.control_metadata_flow_epoch, d.vnic_info_d.epoch
    slt.c1          c1, k.control_metadata_flow_epoch, d.vnic_info_d.epoch
    phvwr.c1.e      p.control_metadata_flow_done, FALSE
    phvwr.c1        p.ingress_recirc_defunct_flow, TRUE
    seq             c1, k.control_metadata_rx_packet, TRUE
    phvwr.c1        p.p4i_i2e_mirror_session, d.vnic_info_d.rx_mirror_session
    phvwr.e         p.p4i_i2e_meter_enabled, d.vnic_info_d.meter_enabled
    phvwr.!c1       p.p4i_i2e_mirror_session, d.vnic_info_d.tx_mirror_session

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
vnic_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

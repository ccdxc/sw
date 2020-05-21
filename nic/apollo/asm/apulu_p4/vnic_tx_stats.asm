#include "apulu.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_vnic_tx_stats_k.h"

struct vnic_tx_stats_k_ k;
struct vnic_tx_stats_d  d;
struct phv_             p;

%%

vnic_tx_stats:
    seq             c1, k.control_metadata_rx_packet, TRUE
    seq.!c1         c1, k.ingress_recirc_valid, TRUE
    seq.!c1         c1, k.arm_to_p4i_valid, TRUE
    tbladd.c1.e.f   d.vnic_tx_stats_d.out_packets, 0
    tbladd.!c1.e    d.vnic_tx_stats_d.out_packets, 1
    tbladd.f        d.vnic_tx_stats_d.out_bytes, k.capri_p4_intrinsic_packet_len

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
vnic_tx_stats_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

#include "apulu.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_vnic_rx_stats_k.h"

struct vnic_rx_stats_k_ k;
struct vnic_rx_stats_d  d;
struct phv_             p;

%%

vnic_rx_stats:
    seq             c1, k.control_metadata_is_local, FALSE
    seq.!c1         c1, k.egress_recirc_mapping_done, FALSE
    seq.!c1         c1, k.p4e_to_arm_valid, TRUE
    tbladd.c1.e.f   d.vnic_rx_stats_d.in_packets, 0
    tbladd.!c1.e    d.vnic_rx_stats_d.in_packets, 1
    tbladd.f        d.vnic_rx_stats_d.in_bytes, k.capri_p4_intrinsic_packet_len

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
vnic_rx_stats_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

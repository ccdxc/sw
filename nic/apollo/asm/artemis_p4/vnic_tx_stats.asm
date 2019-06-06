#include "artemis.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_vnic_tx_stats_k.h"

struct vnic_tx_stats_k_ k;
struct vnic_tx_stats_d  d;
struct phv_ p;

%%

vnic_tx_stats:
    tbladd.e        d.vnic_tx_stats_d.out_packets, 1
    tbladd.f        d.vnic_tx_stats_d.out_packets, k.capri_p4_intrinsic_packet_len

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
vnic_tx_stats_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

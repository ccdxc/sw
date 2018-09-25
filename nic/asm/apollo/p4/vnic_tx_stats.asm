#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct vnic_tx_stats_k   k;
struct vnic_tx_stats_d   d;
struct phv_     p;

%%

vnic_tx_stats:
    tbladd.e        d.vnic_tx_stats_d.out_packets, 1
    tbladd          d.vnic_tx_stats_d.out_bytes, \
                    k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                       capri_p4_intrinsic_packet_len_sbit6_ebit13}

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
vnic_tx_stats_error:
    nop.e
    nop

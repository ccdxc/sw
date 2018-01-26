#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct tx_vport_stats_k k;
struct tx_vport_stats_d d;
struct phv_ p;

%%

tx_vport_stats:
    phvwr           p.capri_p4_intrinsic_valid, TRUE
    seq             c1, k.capri_intrinsic_drop, TRUE
    bcf             [c1], tx_vport_stats_drop
    tbladd.!c1      d.tx_vport_stats_d.permit_packets, 1
    tbladd.e        d.tx_vport_stats_d.permit_bytes, \
                        k.{capri_p4_intrinsic_packet_len_sbit0_ebit5,\
                           capri_p4_intrinsic_packet_len_sbit6_ebit13}
    nop

tx_vport_stats_drop:
    tbladd.e        d.tx_vport_stats_d.drop_packets, 1
    tbladd          d.tx_vport_stats_d.drop_bytes, \
                        k.{capri_p4_intrinsic_packet_len_sbit0_ebit5,\
                           capri_p4_intrinsic_packet_len_sbit6_ebit13}

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tx_vport_stats_error:
    nop.e
    nop

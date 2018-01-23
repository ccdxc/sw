#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct rx_vport_stats_k k;
struct rx_vport_stats_d d;
struct phv_ p;

%%

rx_vport_stats:
    phvwr           p.capri_p4_intrinsic_valid, TRUE
    seq             c1, k.capri_intrinsic_drop, TRUE
    bcf             [c1], rx_vport_stats_drop
    tbladd.!c1      d.rx_vport_stats_d.permit_packets, 1
    tbladd.e        d.rx_vport_stats_d.permit_bytes, \
                        k.{capri_p4_intrinsic_packet_len_sbit0_ebit5,\
                           capri_p4_intrinsic_packet_len_sbit6_ebit13}
    nop

rx_vport_stats_drop:
    tbladd.e        d.rx_vport_stats_d.drop_packets, 1
    tbladd          d.rx_vport_stats_d.drop_bytes, \
                        k.{capri_p4_intrinsic_packet_len_sbit0_ebit5,\
                           capri_p4_intrinsic_packet_len_sbit6_ebit13}

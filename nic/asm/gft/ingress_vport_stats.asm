#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct ingress_vport_stats_k k;
struct ingress_vport_stats_d d;
struct phv_ p;

%%

ingress_vport_stats:
    seq             c1, k.capri_intrinsic_drop, TRUE
    bcf             [c1], ingress_vport_stats_drop
    tbladd.!c1      d.ingress_vport_stats_d.permit_packets, 1
    tbladd.e        d.ingress_vport_stats_d.permit_bytes, \
                        k.{capri_p4_intrinsic_packet_len_sbit0_ebit5,\
                           capri_p4_intrinsic_packet_len_sbit6_ebit13}
    nop

ingress_vport_stats_drop:
    tbladd.e        d.ingress_vport_stats_d.drop_packets, 1
    tbladd          d.ingress_vport_stats_d.drop_bytes, \
                        k.{capri_p4_intrinsic_packet_len_sbit0_ebit5,\
                           capri_p4_intrinsic_packet_len_sbit6_ebit13}

#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct hdr_transpositions3_k k;
struct hdr_transpositions3_d d;
struct phv_ p;

%%

l4_hdr_transpositions:
    // flow stats
    tbladd          d.{l4_hdr_transpositions_d.in_pkts}.dx, 1
    tbladd          d.{l4_hdr_transpositions_d.in_bytes}.dx, \
                        k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                           capri_p4_intrinsic_packet_len_sbit6_ebit13}

    sne             c1, d.l4_hdr_transpositions_d.hdr_bits[7:0], 0
    balcf           r7, [c1], l4_hdr_transpositions_layer1
    crestore        [c7-c1], d.l4_hdr_transpositions_d.hdr_bits[6:0], 0x3F

    sne             c1, d.l4_hdr_transpositions_d.hdr_bits[15-8], 0
    balcf           r7, [c1], l4_hdr_transpositions_layer2
    crestore        [c7-c1], d.l4_hdr_transpositions_d.hdr_bits[14:8], 0x3F

    sne             c1, d.l4_hdr_transpositions_d.hdr_bits[23:16], 0
    balcf           r7, [c1], l4_hdr_transpositions_layer3
    crestore        [c7-c1], d.l4_hdr_transpositions_d.hdr_bits[22:16], 0x3F

    sne             c1, d.l4_hdr_transpositions_d.hdr_bits[9:5], 0
    nop.!c1.e
    crestore        [c4-c1], d.l4_hdr_transpositions_d.hdr_bits[3:0], 0xF

l4_hdr_transpositions_layer0:
    // vxlan_00
    phvwr.c1        p.vxlan_00_valid, TRUE
    phvwr.c1        p.vxlan_00_vni, d.l4_hdr_transpositions_d.tenant_id_00
    phvwri.c1       p.{vxlan_00_flags,vxlan_00_reserved}, 0x08000000

    // udp_00
    phvwr.c2        p.udp_00_valid, TRUE
    phvwr.c2        p.{udp_00_srcPort,udp_00_dstPort}, \
                        d.{l4_hdr_transpositions_d.l4_sport_00,\
                           l4_hdr_transpositions_d.l4_dport_00}
    add             r1, k.{capri_p4_intrinsic_packet_len_sbit0_ebit5,\
                           capri_p4_intrinsic_packet_len_sbit6_ebit13}, \
                        d.l4_hdr_transpositions_d.encap_len_00
    phvwr.c3        p.udp_00_len, r1

    // vxlan_01
    phvwr.c3        p.vxlan_01_valid, TRUE
    phvwr.c3        p.vxlan_01_vni, d.l4_hdr_transpositions_d.tenant_id_01
    phvwri.c3       p.{vxlan_01_flags,vxlan_01_reserved}, 0x08000000

    // udp_01
    phvwr.c4        p.udp_01_valid, TRUE
    phvwr.c4        p.udp_01_srcPort, d.l4_hdr_transpositions_d.l4_sport_01
    phvwr.c4        p.udp_01_dstPort, d.l4_hdr_transpositions_d.l4_dport_01
    add.e           r1, k.{capri_p4_intrinsic_packet_len_sbit0_ebit5,\
                           capri_p4_intrinsic_packet_len_sbit6_ebit13}, \
                        d.l4_hdr_transpositions_d.encap_len_01
    phvwr.c4        p.udp_01_len, r1

l4_hdr_transpositions_layer1:
    phvwr.c1        p.vxlan_1_valid, FALSE
    phvwr.c2        p.udp_1_valid, FALSE
    phvwr.c3        p.udp_1_srcPort, d.l4_hdr_transpositions_d.l4_sport_1
    phvwr.c4        p.udp_1_dstPort, d.l4_hdr_transpositions_d.l4_dport_1
    phvwr.c5        p.tcp_1_srcPort, d.l4_hdr_transpositions_d.l4_sport_1
    phvwr.c6        p.tcp_1_dstPort, d.l4_hdr_transpositions_d.l4_dport_1
    phvwr.c7        p.icmp_1_icmp_type, d.l4_hdr_transpositions_d.l4_sport_1
    crestore        [c1], d.l4_hdr_transpositions_d.hdr_bits[7], 0x1
    jr              r7
    phvwr.c1        p.icmp_1_icmp_code, d.l4_hdr_transpositions_d.l4_dport_1

l4_hdr_transpositions_layer2:
    phvwr.c1        p.vxlan_2_valid, FALSE
    phvwr.c2        p.udp_2_valid, FALSE
    phvwr.c3        p.udp_2_srcPort, d.l4_hdr_transpositions_d.l4_sport_2
    phvwr.c4        p.udp_2_dstPort, d.l4_hdr_transpositions_d.l4_dport_2
    phvwr.c5        p.tcp_2_srcPort, d.l4_hdr_transpositions_d.l4_sport_2
    phvwr.c6        p.tcp_2_dstPort, d.l4_hdr_transpositions_d.l4_dport_2
    phvwr.c7        p.icmp_2_icmp_type, d.l4_hdr_transpositions_d.l4_sport_2
    crestore        [c1], d.l4_hdr_transpositions_d.hdr_bits[15], 0x1
    jr              r7
    phvwr.c1        p.icmp_2_icmp_code, d.l4_hdr_transpositions_d.l4_dport_2

l4_hdr_transpositions_layer3:
    phvwr.c1        p.vxlan_3_valid, FALSE
    phvwr.c2        p.udp_3_valid, FALSE
    phvwr.c3        p.udp_3_srcPort, d.l4_hdr_transpositions_d.l4_sport_3
    phvwr.c4        p.udp_3_dstPort, d.l4_hdr_transpositions_d.l4_dport_3
    phvwr.c5        p.tcp_3_srcPort, d.l4_hdr_transpositions_d.l4_sport_3
    phvwr.c6        p.tcp_3_dstPort, d.l4_hdr_transpositions_d.l4_dport_3
    phvwr.c7        p.icmp_3_icmp_type, d.l4_hdr_transpositions_d.l4_sport_3
    crestore        [c1], d.l4_hdr_transpositions_d.hdr_bits[23], 0x1
    jr              r7
    phvwr.c1        p.icmp_3_icmp_code, d.l4_hdr_transpositions_d.l4_dport_3

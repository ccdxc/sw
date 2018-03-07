#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct rx_hdr_transpositions3_k k;
struct rx_hdr_transpositions3_d d;
struct phv_ p;

%%

rx_l4_hdr_transpositions:
    // flow stats
    tbladd          d.{rx_l4_hdr_transpositions_d.in_pkts}.dx, 1
    tbladd.f        d.{rx_l4_hdr_transpositions_d.in_bytes}.dx, \
                        k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                           capri_p4_intrinsic_packet_len_sbit6_ebit13}

    sne             c1, d.rx_l4_hdr_transpositions_d.hdr_bits[15:8], 0
    balcf           r7, [c1], rx_l4_hdr_transpositions_layer2
    crestore        [c7-c1], d.rx_l4_hdr_transpositions_d.hdr_bits[14:8], 0x7F

    sne             c1, d.rx_l4_hdr_transpositions_d.hdr_bits[23:16], 0
    balcf           r7, [c1], rx_l4_hdr_transpositions_layer3
    crestore        [c7-c1], d.rx_l4_hdr_transpositions_d.hdr_bits[22:16], 0x7F

    sne             c1, d.rx_l4_hdr_transpositions_d.hdr_bits[7:0], 0
    nop.!c1.e
    crestore        [c7-c1], d.rx_l4_hdr_transpositions_d.hdr_bits[6:0], 0x7F

rx_l4_hdr_transpositions_layer1:
    bcf             [!c1&!c2&!c3&!c4&!c5&!c6&!c7], rx_l4_hdr_transpositions_layer1_next
    phvwr.c1        p.vxlan_1_valid, FALSE
    phvwr.c2        p.udp_1_valid, FALSE
    phvwr.c3        p.udp_1_srcPort, d.rx_l4_hdr_transpositions_d.l4_sport_1
    phvwr.c4        p.udp_1_dstPort, d.rx_l4_hdr_transpositions_d.l4_dport_1
    phvwr.c5        p.tcp_1_srcPort, d.rx_l4_hdr_transpositions_d.l4_sport_1
    phvwr.c6        p.tcp_1_dstPort, d.rx_l4_hdr_transpositions_d.l4_dport_1
    phvwr.c7        p.icmp_1_icmp_type, d.rx_l4_hdr_transpositions_d.l4_sport_1
    setcf           c7, [c3|c4|c5|c6]
    phvwr.c7        p.control_metadata_update_checksum_1, TRUE
rx_l4_hdr_transpositions_layer1_next:
    crestore.e      [c1], d.rx_l4_hdr_transpositions_d.hdr_bits[7], 0x1
    phvwr.c1        p.icmp_1_icmp_code, d.rx_l4_hdr_transpositions_d.l4_dport_1

rx_l4_hdr_transpositions_layer2:
    bcf             [!c1&!c2&!c3&!c4&!c5&!c6&!c7], rx_l4_hdr_transpositions_layer2_next
    phvwr.c1        p.vxlan_2_valid, FALSE
    phvwr.c2        p.udp_2_valid, FALSE
    phvwr.c3        p.udp_2_srcPort, d.rx_l4_hdr_transpositions_d.l4_sport_2
    phvwr.c4        p.udp_2_dstPort, d.rx_l4_hdr_transpositions_d.l4_dport_2
    phvwr.c5        p.tcp_2_srcPort, d.rx_l4_hdr_transpositions_d.l4_sport_2
    phvwr.c6        p.tcp_2_dstPort, d.rx_l4_hdr_transpositions_d.l4_dport_2
    phvwr.c7        p.icmp_2_icmp_type, d.rx_l4_hdr_transpositions_d.l4_sport_2
    setcf           c7, [c3|c4|c5|c6]
    phvwr.c7        p.control_metadata_update_checksum_2, TRUE
rx_l4_hdr_transpositions_layer2_next:
    crestore        [c1], d.rx_l4_hdr_transpositions_d.hdr_bits[15], 0x1
    jr              r7
    phvwr.c1        p.icmp_2_icmp_code, d.rx_l4_hdr_transpositions_d.l4_dport_2

rx_l4_hdr_transpositions_layer3:
    bcf             [!c1&!c2&!c3&!c4&!c5&!c6&!c7], rx_l4_hdr_transpositions_layer3_next
    phvwr.c2        p.udp_3_valid, FALSE
    phvwr.c3        p.udp_3_srcPort, d.rx_l4_hdr_transpositions_d.l4_sport_3
    phvwr.c4        p.udp_3_dstPort, d.rx_l4_hdr_transpositions_d.l4_dport_3
    phvwr.c5        p.tcp_3_srcPort, d.rx_l4_hdr_transpositions_d.l4_sport_3
    phvwr.c6        p.tcp_3_dstPort, d.rx_l4_hdr_transpositions_d.l4_dport_3
    phvwr.c7        p.icmp_3_icmp_type, d.rx_l4_hdr_transpositions_d.l4_sport_3
    setcf           c7, [c3|c4|c5|c6]
    phvwr.c7        p.control_metadata_update_checksum_3, TRUE
rx_l4_hdr_transpositions_layer3_next:
    crestore        [c1], d.rx_l4_hdr_transpositions_d.hdr_bits[23], 0x1
    jr              r7
    phvwr.c1        p.icmp_3_icmp_code, d.rx_l4_hdr_transpositions_d.l4_dport_3

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rx_hdr_transpositions3_error:
    nop.e
    nop

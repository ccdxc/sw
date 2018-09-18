#include "tyche.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct rx_rewrite2_k k;
struct rx_rewrite2_d d;
struct phv_ p;

%%

rx_l4_rewrite:
    // flow stats
    tbladd          d.{rx_l4_rewrite_d.in_packets}.dx, 1
    tbladd.f        d.{rx_l4_rewrite_d.in_bytes}.dx, \
                        k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                           capri_p4_intrinsic_packet_len_sbit6_ebit13}

    sne             c1, d.rx_l4_rewrite_d.hdr_bits[15:10], 0
    balcf           r7, [c1], rx_l4_rewrite_layer2
    crestore        [c6-c1], d.rx_l4_rewrite_d.hdr_bits[15:10], 0x3F

    sne             c1, d.rx_l4_rewrite_d.hdr_bits[9:4], 0
    balcf           r7, [c1], rx_l4_rewrite_layer1
    crestore        [c6-c1], d.rx_l4_rewrite_d.hdr_bits[9:4], 0x3F

    sne             c1, d.rx_l4_rewrite_d.hdr_bits[3:0], 0
    nop.!c1.e
    crestore        [c4-c1], d.rx_l4_rewrite_d.hdr_bits[3:0], 0xF

rx_l4_rewrite_layer0:
    phvwr.c1        p.vxlan_1_valid, FALSE
    phvwr.c2        p.udp_1_valid, FALSE
    phvwr.c3        p.gre_1_valid, FALSE
    nop.e
    phvwr.c4        p.{mpls_2_valid,mpls_1_valid,mpls_0_valid}, 0

rx_l4_rewrite_layer1:
    phvwr.c1        p.udp_1_srcPort, d.rx_l4_rewrite_d.l4_sport_1
    phvwr.c2        p.udp_1_dstPort, d.rx_l4_rewrite_d.l4_dport_1
    phvwr.c3        p.tcp_1_srcPort, d.rx_l4_rewrite_d.l4_sport_1
    phvwr.c4        p.tcp_1_dstPort, d.rx_l4_rewrite_d.l4_dport_1
    phvwr.c5        p.icmp_1_icmp_type, d.rx_l4_rewrite_d.l4_sport_1
    phvwr.c6        p.icmp_1_icmp_code, d.rx_l4_rewrite_d.l4_dport_1
    setcf           c7, [c1|c2|c3|c4]
    jr              r7
    phvwr.c7        p.control_metadata_update_checksum_1, TRUE

rx_l4_rewrite_layer2:
    phvwr.c1        p.udp_2_srcPort, d.rx_l4_rewrite_d.l4_sport_2
    phvwr.c2        p.udp_2_dstPort, d.rx_l4_rewrite_d.l4_dport_2
    phvwr.c3        p.tcp_2_srcPort, d.rx_l4_rewrite_d.l4_sport_2
    phvwr.c4        p.tcp_2_dstPort, d.rx_l4_rewrite_d.l4_dport_2
    phvwr.c5        p.icmp_2_icmp_type, d.rx_l4_rewrite_d.l4_sport_2
    phvwr.c6        p.icmp_2_icmp_code, d.rx_l4_rewrite_d.l4_dport_2
    setcf           c7, [c1|c2|c3|c4]
    jr              r7
    phvwr.c7        p.control_metadata_update_checksum_2, TRUE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rx_rewrite2_error:
    nop.e
    nop

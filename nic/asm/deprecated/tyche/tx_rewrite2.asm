#include "tyche.h"
#include "egress.h"
#include "EGRESS_p.h"

struct tx_rewrite2_k k;
struct tx_rewrite2_d d;
struct phv_ p;

%%

tx_l4_rewrite:
    // flow stats
    tbladd          d.{tx_l4_rewrite_d.in_pkts}.dx, 1
    tbladd.f        d.{tx_l4_rewrite_d.in_bytes}.dx, \
                        k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                           capri_p4_intrinsic_packet_len_sbit6_ebit13}

    sne             c1, d.tx_l4_rewrite_d.hdr1_bits[9:4], 0
    balcf           r7, [c1], tx_l4_rewrite_layer1
    crestore        [c6-c1], d.tx_l4_rewrite_d.hdr1_bits[9:4], 0x3F

    sne             c1, d.tx_l4_rewrite_d.hdr0_bits[3:0], 0
    nop.!c1.e
    crestore        [c4-c1], d.tx_l4_rewrite_d.hdr0_bits[3:0], 0xF

tx_l4_rewrite_layer0:
    // vxlan
    phvwr.c1        p.vxlan_0_valid, TRUE
    phvwr.c1        p.vxlan_0_vni, d.tx_l4_rewrite_d.tenant_id_0
    phvwri.c1       p.{vxlan_0_flags,vxlan_0_reserved}, 0x08000000

    // gre
    phvwr.c2        p.gre_0_valid, TRUE
    phvwr.c2        p.gre_0_proto, d.tx_l4_rewrite_d.gre_proto_0

    // udp
    phvwr.c3        p.udp_0_valid, TRUE
    phvwr.c3        p.{udp_0_srcPort,udp_0_dstPort}, \
                        d.{tx_l4_rewrite_d.l4_sport_0,\
                           tx_l4_rewrite_d.l4_dport_0}
    add             r1, k.{capri_p4_intrinsic_packet_len_sbit0_ebit5,\
                           capri_p4_intrinsic_packet_len_sbit6_ebit13}, \
                        d.tx_l4_rewrite_d.encap_len_0
    phvwr.c3        p.udp_0_len, r1

    // mpls
    nop.!c4.e
    crestore        [c3-c1], d.{tx_l4_rewrite_d.mpls0_valid, \
                                tx_l4_rewrite_d.mpls1_valid, \
                                tx_l4_rewrite_d.mpls2_valid}, 0x7
    phvwr.c3        p.{mpls_0_label...mpls_0_ttl}, \
                        d.{tx_l4_rewrite_d.mpls0_label...tx_l4_rewrite_d.mpls0_ttl}
    phvwr.c2        p.{mpls_1_label...mpls_1_ttl}, \
                        d.{tx_l4_rewrite_d.mpls1_label...tx_l4_rewrite_d.mpls1_ttl}
    nop.e
    phvwr.c1        p.{mpls_2_label...mpls_2_ttl}, \
                        d.{tx_l4_rewrite_d.mpls2_label...tx_l4_rewrite_d.mpls2_ttl}

tx_l4_rewrite_layer1:
    phvwr.c1        p.udp_1_srcPort, d.tx_l4_rewrite_d.l4_sport_1
    phvwr.c2        p.udp_1_dstPort, d.tx_l4_rewrite_d.l4_dport_1
    phvwr.c3        p.tcp_1_srcPort, d.tx_l4_rewrite_d.l4_sport_1
    phvwr.c4        p.tcp_1_dstPort, d.tx_l4_rewrite_d.l4_dport_1
    phvwr.c5        p.icmp_1_icmp_type, d.tx_l4_rewrite_d.l4_sport_1
    jr              r7
    phvwr.c6        p.icmp_1_icmp_code, d.tx_l4_rewrite_d.l4_dport_1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tx_rewrite2_error:
    nop.e
    nop

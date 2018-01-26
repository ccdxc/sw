#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct tx_hdr_transpositions3_k k;
struct tx_hdr_transpositions3_d d;
struct phv_ p;

%%

tx_l4_hdr_transpositions:
    // flow stats
    tbladd          d.{tx_l4_hdr_transpositions_d.in_pkts}.dx, 1
    tbladd          d.{tx_l4_hdr_transpositions_d.in_bytes}.dx, \
                        k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                           capri_p4_intrinsic_packet_len_sbit6_ebit13}

    sne             c1, d.tx_l4_hdr_transpositions_d.hdr_bits[7:2], 0
    balcf           r7, [c1], tx_l4_hdr_transpositions_layer1
    crestore        [c7-c1], d.tx_l4_hdr_transpositions_d.hdr_bits[7:2], 0x3F

    sne             c1, d.tx_l4_hdr_transpositions_d.hdr0_bits[3:0], 0
    nop.!c1.e
    crestore        [c4-c1], d.tx_l4_hdr_transpositions_d.hdr0_bits[3:0], 0xF

tx_l4_hdr_transpositions_layer0:
    // vxlan_00
    phvwr.c1        p.vxlan_00_valid, TRUE
    phvwr.c1        p.vxlan_00_vni, d.tx_l4_hdr_transpositions_d.tenant_id_00
    phvwri.c1       p.{vxlan_00_flags,vxlan_00_reserved}, 0x08000000

    // udp_00
    phvwr.c2        p.udp_00_valid, TRUE
    phvwr.c2        p.{udp_00_srcPort,udp_00_dstPort}, \
                        d.{tx_l4_hdr_transpositions_d.l4_sport_00,\
                           tx_l4_hdr_transpositions_d.l4_dport_00}
    add             r1, k.{capri_p4_intrinsic_packet_len_sbit0_ebit5,\
                           capri_p4_intrinsic_packet_len_sbit6_ebit13}, \
                        d.tx_l4_hdr_transpositions_d.encap_len_00
    phvwr.c2        p.udp_00_len, r1

    // vxlan_01
    phvwr.c3        p.vxlan_01_valid, TRUE
    phvwr.c3        p.vxlan_01_vni, d.tx_l4_hdr_transpositions_d.tenant_id_01
    phvwri.c3       p.{vxlan_01_flags,vxlan_01_reserved}, 0x08000000

    // udp_01
    phvwr.c4        p.udp_01_valid, TRUE
    phvwr.c4        p.udp_01_srcPort, d.tx_l4_hdr_transpositions_d.l4_sport_01
    phvwr.c4        p.udp_01_dstPort, d.tx_l4_hdr_transpositions_d.l4_dport_01
    add.e           r1, k.{capri_p4_intrinsic_packet_len_sbit0_ebit5,\
                           capri_p4_intrinsic_packet_len_sbit6_ebit13}, \
                        d.tx_l4_hdr_transpositions_d.encap_len_01
    phvwr.c4        p.udp_01_len, r1

tx_l4_hdr_transpositions_layer1:
    phvwr.c1        p.udp_1_srcPort, d.tx_l4_hdr_transpositions_d.l4_sport_1
    phvwr.c2        p.udp_1_dstPort, d.tx_l4_hdr_transpositions_d.l4_dport_1
    phvwr.c3        p.tcp_1_srcPort, d.tx_l4_hdr_transpositions_d.l4_sport_1
    phvwr.c4        p.tcp_1_dstPort, d.tx_l4_hdr_transpositions_d.l4_dport_1
    phvwr.c5        p.icmp_1_icmp_type, d.tx_l4_hdr_transpositions_d.l4_sport_1
    jr              r7
    phvwr.c6        p.icmp_1_icmp_code, d.tx_l4_hdr_transpositions_d.l4_dport_1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tx_hdr_transpositions3_error:
    nop.e
    nop

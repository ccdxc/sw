#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct rx_key4_k k;
struct rx_key4_d d;
struct phv_ p;

%%

rx_key4:

normalize_roce:
check_roce_bth1_valid:
    seq             c7, k.roce_bth_1_valid, TRUE
    bcf             [!c7], check_roce_bth2_valid
    phvwrpair.c7    p.roce_bth_opCode, k.roce_bth_1_opCode, \
                        p.roce_bth_destQP, k.roce_bth_1_destQP
    b               normalize_udp_1
    phvwr           p.roce_metadata_roce_valid, TRUE

check_roce_bth2_valid:
    seq             c7, k.roce_bth_2_valid, TRUE
    bcf             [!c7], check_roce_bth_valid
    phvwrpair.c7    p.roce_bth_opCode, k.roce_bth_2_opCode, \
                        p.roce_bth_destQP, k.roce_bth_2_destQP
    b               normalize_udp_2
    phvwr           p.roce_metadata_roce_valid, TRUE

check_roce_bth_valid:
    bbne            k.roce_bth_valid, TRUE, rx_key4_start
    nop
    phvwr           p.roce_metadata_roce_valid, TRUE

normalize_udp:
    bbne            k.udp_3_valid, TRUE, normalize_udp_2
    add             r7, r0, k.{udp_3_len_sbit0_ebit7,udp_3_len_sbit8_ebit15}
    seq             c2, k.ipv4_3_valid, TRUE
    phvwr.c2        p.roce_metadata_ecn, k.ipv4_3_diffserv[7:6]
    seq             c2, k.ipv6_3_valid, TRUE
    b               rx_key4_start
    phvwr.c2        p.roce_metadata_ecn, k.ipv6_3_trafficClass_sbit0_ebit3[3:2]

normalize_udp_2:
    bbne            k.udp_2_valid, TRUE, normalize_udp_1
    add             r7, r0, k.{udp_2_len_sbit0_ebit7,udp_2_len_sbit8_ebit15}
    seq             c2, k.ipv4_2_valid, TRUE
    phvwr.c2        p.roce_metadata_ecn, k.ipv4_2_diffserv[7:6]
    seq             c2, k.ipv6_2_valid, TRUE
    b               rx_key4_start
    phvwr.c2        p.roce_metadata_ecn, k.ipv6_2_trafficClass_sbit0_ebit3[3:2]

normalize_udp_1:
    bbne            k.udp_1_valid, TRUE, rx_key4_start
    add             r7, r0, k.{udp_1_len_sbit0_ebit7,udp_1_len_sbit8_ebit15}
    seq             c2, k.ipv4_1_valid, TRUE
    phvwr.c2        p.roce_metadata_ecn, k.ipv4_1_diffserv[7:6]
    seq             c2, k.ipv6_1_valid, TRUE
    b               rx_key4_start
    phvwr.c2        p.roce_metadata_ecn, k.ipv6_1_trafficClass_sbit0_ebit3[3:2]

rx_key4_start:
    phvwr           p.roce_metadata_udp_len, r7
    nop.!c1.e
    crestore        [c7-c1], d.rx_key4_d.match_fields[6:0], 0x7F
    add             r7, r0, r0
    bcf             [!c1&!c2&!c3&!c4&!c5&!c6&!c7], rx_key_start_2
    or.c1           r7, r7, k.l4_metadata_l4_sport_1, 16
    or.c2           r7, r7, k.l4_metadata_l4_dport_1
    phvwr.c3        p.flow_lkp_metadata_tcp_flags_1, k.tcp_1_flags
    or.c4           r7, r7, k.icmp_1_icmp_type, 16
    or.c5           r7, r7, k.icmp_1_icmp_code
    phvwr.c6        p.flow_lkp_metadata_tenant_id_1, k.tunnel_metadata_tunnel_vni_1
    phvwr.c7        p.flow_lkp_metadata_gre_proto_1, k.gre_1_proto
    phvwr           p.{flow_lkp_metadata_l4_sport_1,flow_lkp_metadata_l4_dport_1}, r7

rx_key_start_2:
    crestore        [c7-c1], d.rx_key4_d.match_fields[13:7], 0x7F
    add             r7, r0, r0
    bcf             [!c1&!c2&!c3&!c4&!c5&!c6&!c7], rx_key_start_3
    or.c1           r7, r7, k.l4_metadata_l4_sport_2, 16
    or.c2           r7, r7, k.l4_metadata_l4_dport_2
    phvwr.c3        p.flow_lkp_metadata_tcp_flags_2, k.tcp_2_flags
    or.c4           r7, r7, k.icmp_2_icmp_type, 16
    or.c5           r7, r7, k.icmp_2_icmp_code
    phvwr.c6        p.flow_lkp_metadata_tenant_id_2, k.tunnel_metadata_tunnel_vni_2
    phvwr.c7        p.flow_lkp_metadata_gre_proto_2, k.gre_2_proto
    phvwr           p.{flow_lkp_metadata_l4_sport_2,flow_lkp_metadata_l4_dport_2}, r7

rx_key_start_3:
    seq             c1, d.rx_key4_d.match_fields[20:14], r0
    nop.c1.e
    crestore        [c7-c1], d.rx_key4_d.match_fields[20:14], 0x7F
    add             r7, r0, r0
    or.c1           r7, r7, k.l4_metadata_l4_sport_3, 16
    or.c2           r7, r7, k.l4_metadata_l4_dport_3
    phvwr.c3        p.flow_lkp_metadata_tcp_flags_3, k.tcp_3_flags
    or.c4           r7, r7, k.icmp_3_icmp_type, 16
    or.c5           r7, r7, k.icmp_3_icmp_code
    phvwr.c6        p.flow_lkp_metadata_tenant_id_3, k.tunnel_metadata_tunnel_vni_3
    phvwr.c7        p.flow_lkp_metadata_gre_proto_3, k.gre_3_proto
    nop.e
    phvwr.f         p.{flow_lkp_metadata_l4_sport_3,flow_lkp_metadata_l4_dport_3}, r7

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rx_key4_error:
    nop.e
    nop

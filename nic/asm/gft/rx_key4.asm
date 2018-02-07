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
    bbne        k.roce_bth_1_valid, TRUE, check_roce_bth2_valid
    phvwr       p.capri_rxdma_intrinsic_qid, k.roce_bth_1_destQP
    b           normalize_udp
    phvwr       p.roce_metadata_roce_valid, TRUE

check_roce_bth2_valid:
    bbne        k.roce_bth_2_valid, TRUE, check_roce_bth_valid
    phvwr       p.capri_rxdma_intrinsic_qid, k.roce_bth_2_destQP
    b           normalize_udp
    phvwr       p.roce_metadata_roce_valid, TRUE

check_roce_bth_valid:
    bbne        k.roce_bth_valid, TRUE, rx_key4_start
    phvwr       p.capri_rxdma_intrinsic_qid, k.roce_bth_destQP
    phvwr       p.roce_metadata_roce_valid, TRUE

normalize_udp:
check_udp_3_valid:
    bbne        k.udp_3_valid, TRUE, check_udp_2_valid
    phvwr       p.roce_metadata_udp_len, \
                    k.{udp_3_len_sbit0_ebit7,udp_3_len_sbit8_ebit15}
    seq         c2, k.ipv4_3_valid, TRUE
    phvwr.c2    p.roce_metadata_ecn, k.ipv4_3_diffserv[7:6]
    seq         c2, k.ipv6_3_valid, TRUE
    b           rx_key4_start
    phvwr.c2    p.roce_metadata_ecn, k.ipv6_3_trafficClass_sbit0_ebit3[3:2]

check_udp_2_valid:
    bbne        k.udp_2_valid, TRUE, check_udp_1_valid
    phvwr       p.roce_metadata_udp_len, \
                    k.{udp_2_len_sbit0_ebit7,udp_2_len_sbit8_ebit15}
    seq         c2, k.ipv4_2_valid, TRUE
    phvwr.c2    p.roce_metadata_ecn, k.ipv4_2_diffserv[7:6]
    seq         c2, k.ipv6_2_valid, TRUE
    b           rx_key4_start
    phvwr.c2    p.roce_metadata_ecn, k.ipv6_2_trafficClass_sbit0_ebit3[3:2]

check_udp_1_valid:
    bbne        k.udp_1_valid, TRUE, rx_key4_start
    phvwr       p.roce_metadata_udp_len, \
                    k.{udp_1_len_sbit0_ebit7,udp_1_len_sbit8_ebit15}
    seq         c2, k.ipv4_1_valid, TRUE
    phvwr.c2    p.roce_metadata_ecn, k.ipv4_1_diffserv[7:6]
    seq         c2, k.ipv6_1_valid, TRUE
    b           rx_key4_start
    phvwr.c2    p.roce_metadata_ecn, k.ipv6_1_trafficClass_sbit0_ebit3[3:2]

rx_key4_start:
    nop.!c1.e
    crestore    [c7-c1], d.rx_key4_d.match_fields[6:0], 0x7F
    phvwr.c1    p.flow_lkp_metadata_l4_sport_1, k.l4_metadata_l4_sport_1
    phvwr.c2    p.flow_lkp_metadata_l4_dport_1, k.l4_metadata_l4_dport_1
    phvwr.c3    p.flow_lkp_metadata_tcp_flags_1, k.tcp_1_flags
    phvwr.c4    p.flow_lkp_metadata_l4_sport_1, k.icmp_1_icmp_type
    phvwr.c5    p.flow_lkp_metadata_l4_dport_1, k.icmp_1_icmp_code
    phvwr.c6    p.flow_lkp_metadata_tenant_id_1, k.tunnel_metadata_tunnel_vni_1
    phvwr.c7    p.flow_lkp_metadata_gre_proto_1, k.gre_1_proto

    crestore    [c7-c1], d.rx_key4_d.match_fields[13:7], 0x7F
    phvwr.c1    p.flow_lkp_metadata_l4_sport_2, k.l4_metadata_l4_sport_2
    phvwr.c2    p.flow_lkp_metadata_l4_dport_2, k.l4_metadata_l4_dport_2
    phvwr.c3    p.flow_lkp_metadata_tcp_flags_2, k.tcp_2_flags
    phvwr.c4    p.flow_lkp_metadata_l4_sport_2, k.icmp_2_icmp_type
    phvwr.c5    p.flow_lkp_metadata_l4_dport_2, k.icmp_2_icmp_code
    phvwr.c6    p.flow_lkp_metadata_tenant_id_2, k.tunnel_metadata_tunnel_vni_2
    phvwr.c7    p.flow_lkp_metadata_gre_proto_2, k.gre_2_proto

    crestore    [c7-c1], d.rx_key4_d.match_fields[20:14], 0x7F
    phvwr.c1    p.flow_lkp_metadata_l4_sport_3, k.l4_metadata_l4_sport_3
    phvwr.c2    p.flow_lkp_metadata_l4_dport_3, k.l4_metadata_l4_dport_3
    phvwr.c3    p.flow_lkp_metadata_tcp_flags_3, k.tcp_3_flags
    phvwr.c4    p.flow_lkp_metadata_l4_sport_3, k.icmp_3_icmp_type
    phvwr.c5    p.flow_lkp_metadata_l4_dport_3, k.icmp_3_icmp_code
    phvwr.c6    p.flow_lkp_metadata_tenant_id_3, k.tunnel_metadata_tunnel_vni_3
    nop.e
    phvwr.c7    p.flow_lkp_metadata_gre_proto_3, k.gre_3_proto

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rx_key4_error:
    nop.e
    nop

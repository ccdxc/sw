#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct tx_key_k k;
struct tx_key_d d;
struct phv_ p;

%%

tx_key:
    nop.!c1.e
    phvwr       p.flow_action_metadata_tx_ethernet_dst, k.ethernet_1_dstAddr

    crestore    [c7-c1], d.tx_key_d.match_fields[31:16], 0x7F
    phvwr.c1    p.flow_lkp_metadata_l4_sport_1, k.l4_metadata_l4_sport_1
    phvwr.c2    p.flow_lkp_metadata_l4_dport_1, k.l4_metadata_l4_dport_1
    phvwr.c3    p.flow_lkp_metadata_tcp_flags_1, k.tcp_1_flags
    phvwr.c4    p.flow_lkp_metadata_l4_sport_1, k.icmp_1_icmp_type
    phvwr.c5    p.flow_lkp_metadata_l4_dport_1, k.icmp_1_icmp_code

    crestore    [c4-c1], d.tx_key_d.match_fields[3:0], 0xF
    phvwr.c1    p.flow_lkp_metadata_ethernet_dst_1, k.ethernet_1_dstAddr
    phvwr.c2    p.flow_lkp_metadata_ethernet_src_1, k.ethernet_1_srcAddr
    phvwr.c3    p.flow_lkp_metadata_ethernet_type_1, k.ethernet_1_etherType
    phvwr.c4    p.flow_lkp_metadata_ctag_1, \
                    k.{ctag_1_vid_sbit0_ebit3,ctag_1_vid_sbit4_ebit11}

    bbeq        k.ipv4_1_valid, TRUE, tx_key_ipv4
    crestore    [c5-c1], d.tx_key_d.match_fields[8:4], 0x1F
    bbeq        k.ipv6_1_valid, TRUE, tx_key_ipv6
    nop
    nop.e
    nop

tx_key_ipv4:
    phvwr.c1    p.flow_lkp_metadata_ip_src_1, k.ipv4_1_srcAddr
    phvwr.c2    p.flow_lkp_metadata_ip_dst_1, k.ipv4_1_dstAddr
    phvwr.c3    p.flow_lkp_metadata_ip_dscp_1, k.ipv4_1_diffserv
    phvwr.c4    p.flow_lkp_metadata_ip_proto_1, k.ipv4_1_protocol
    nop.e
    phvwr.c5    p.flow_lkp_metadata_ip_ttl_1, k.ipv4_1_ttl

tx_key_ipv6:
    phvwr.c1    p.flow_lkp_metadata_ip_src_1, \
                    k.{ipv6_1_srcAddr_sbit0_ebit7...ipv6_1_srcAddr_sbit96_ebit127}
    phvwr.c2    p.flow_lkp_metadata_ip_dst_1[127:8], k.ipv6_1_dstAddr_sbit0_ebit119
    phvwr.c2    p.flow_lkp_metadata_ip_dst_1[7:0], k.ipv6_1_dstAddr_sbit120_ebit127
    phvwr.c3    p.flow_lkp_metadata_ip_dscp_1, \
                    k.{ipv6_1_trafficClass_sbit0_ebit3,ipv6_1_trafficClass_sbit4_ebit7}
    phvwr.c4    p.flow_lkp_metadata_ip_proto_1, k.ipv6_1_nextHdr
    nop.e
    phvwr.c5    p.flow_lkp_metadata_ip_ttl_1, k.ipv6_1_hopLimit

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tx_key_error:
    nop.e
    nop

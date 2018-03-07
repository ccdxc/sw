#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct tx_key_k k;
struct tx_key_d d;
struct phv_ p;

%%

tx_key:
    nop.!c1.e
    crestore    [c7-c1], d.tx_key_d.match_fields[31:16], 0x7F
    add         r7, r0, r0
    or.c1       r7, r7, k.l4_metadata_l4_sport_1, 16
    or.c2       r7, r7, k.l4_metadata_l4_dport_1
    phvwr.c3    p.flow_lkp_metadata_tcp_flags_1, k.tcp_1_flags
    or.c4       r7, r7, k.icmp_1_icmp_type, 16
    or.c5       r7, r7, k.icmp_1_icmp_code
    phvwr       p.{flow_lkp_metadata_l4_sport_1, \
                   flow_lkp_metadata_l4_dport_1}, r7

    crestore    [c4-c1], d.tx_key_d.match_fields[3:0], 0xF
    phvwr.c1    p.flow_lkp_metadata_ethernet_dst_1, k.ethernet_1_dstAddr
    add         r7, r0, r0
    or.c2       r7, r7, k.ethernet_1_srcAddr, 16
    or.c3       r7, r7, k.ethernet_1_etherType
    phvwr       p.{flow_lkp_metadata_ethernet_src_1, \
                   flow_lkp_metadata_ethernet_type_1}, r7
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
    add         r7, r0, r0
    or.c3       r7, r7, k.ipv4_1_diffserv, 16
    or.c4       r7, r7, k.ipv4_1_protocol, 8
    or.c5       r7, r7, k.ipv4_1_ttl
    phvwr.f.e   p.{flow_lkp_metadata_ip_dscp_1,flow_lkp_metadata_ip_proto_1, \
                   flow_lkp_metadata_ip_ttl_1}, r7
    nop

tx_key_ipv6:
    phvwr.c1    p.flow_lkp_metadata_ip_src_1, \
                    k.{ipv6_1_srcAddr_sbit0_ebit31...ipv6_1_srcAddr_sbit64_ebit127}
    phvwr.c2    p.flow_lkp_metadata_ip_dst_1[127:8], k.ipv6_1_dstAddr_sbit0_ebit119
    phvwr.c2    p.flow_lkp_metadata_ip_dst_1[7:0], k.ipv6_1_dstAddr_sbit120_ebit127
    add         r7, r0, r0
    or.c3       r7, r7, k.{ipv6_1_trafficClass_sbit0_ebit3,\
                           ipv6_1_trafficClass_sbit4_ebit7}, 16
    or.c4       r7, r7, k.ipv6_1_nextHdr, 8
    or.c5       r7, r7, k.ipv6_1_hopLimit
    phvwr.f.e   p.{flow_lkp_metadata_ip_dscp_1,flow_lkp_metadata_ip_proto_1, \
                   flow_lkp_metadata_ip_ttl_1}, r7
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tx_key_error:
    nop.e
    nop

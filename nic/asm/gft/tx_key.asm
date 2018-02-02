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

    add         r1, r0, d.tx_key_d.match_fields[31:16]
    seq         c1, r1[MATCH_TRANSPORT_SRC_PORT_1_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_l4_sport_1, k.l4_metadata_l4_sport_1
    seq         c1, r1[MATCH_TRANSPORT_DST_PORT_1_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_l4_dport_1, k.l4_metadata_l4_dport_1
    seq         c1, r1[MATCH_TCP_FLAGS_1_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_tcp_flags_1, k.tcp_1_flags
    seq         c1, r1[MATCH_ICMP_TYPE_1_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_l4_sport_1, k.icmp_1_icmp_type
    seq         c1, r1[MATCH_ICMP_CODE_1_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_l4_dport_1, k.icmp_1_icmp_code

    seq         c1, d.tx_key_d.match_fields[MATCH_ETHERNET_DST_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ethernet_dst_1, k.ethernet_1_dstAddr
    seq         c1, d.tx_key_d.match_fields[MATCH_ETHERNET_SRC_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ethernet_src_1, k.ethernet_1_srcAddr
    seq         c1, d.tx_key_d.match_fields[MATCH_ETHERNET_TYPE_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ethernet_type_1, k.ethernet_1_etherType
    seq         c1, d.tx_key_d.match_fields[MATCH_CUSTOMER_VLAN_ID_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ctag_1, \
                    k.{ctag_1_vid_sbit0_ebit3,ctag_1_vid_sbit4_ebit11}

    seq         c1, k.ipv4_1_valid, TRUE
    bcf         [c1], tx_key_ipv4
    seq         c1, k.ipv6_1_valid, TRUE
    bcf         [c1], tx_key_ipv6

tx_key_ipv4:
    seq         c1, d.tx_key_d.match_fields[MATCH_IP_SRC_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_src_1, k.ipv4_1_srcAddr
    seq         c1, d.tx_key_d.match_fields[MATCH_IP_DST_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_dst_1, k.ipv4_1_dstAddr
    seq         c1, d.tx_key_d.match_fields[MATCH_IP_DSCP_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_dscp_1, k.ipv4_1_diffserv
    seq         c1, d.tx_key_d.match_fields[MATCH_IP_PROTO_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_proto_1, k.ipv4_1_protocol
    seq.e       c1, d.tx_key_d.match_fields[MATCH_IP_TTL_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_ttl_1, k.ipv4_1_ttl

tx_key_ipv6:
    seq         c1, d.tx_key_d.match_fields[MATCH_IP_SRC_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_src_1, \
                    k.{ipv6_1_srcAddr_sbit0_ebit7...ipv6_1_srcAddr_sbit96_ebit127}
    seq         c1, d.tx_key_d.match_fields[MATCH_IP_DST_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_dst_1[127:8], k.ipv6_1_dstAddr_sbit0_ebit119
    phvwr.c1    p.flow_lkp_metadata_ip_dst_1[7:0], k.ipv6_1_dstAddr_sbit120_ebit127
    seq         c1, d.tx_key_d.match_fields[MATCH_IP_DSCP_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_dscp_1, \
                    k.{ipv6_1_trafficClass_sbit0_ebit3,ipv6_1_trafficClass_sbit4_ebit7}
    seq         c1, d.tx_key_d.match_fields[MATCH_IP_PROTO_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_proto_1, k.ipv6_1_nextHdr
    seq.e       c1, d.tx_key_d.match_fields[MATCH_IP_TTL_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_ttl_1, k.ipv6_1_hopLimit

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tx_key_error:
    nop.e
    nop

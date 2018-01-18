#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct ingress_key2_k k;
struct ingress_key2_d d;
struct phv_ p;

%%

ingress_key2:
    nop.!c1.e
    seq         c1, d.ingress_key2_d.match_fields[MATCH_ETHERNET_DST_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ethernet_dst_2, k.ethernet_2_dstAddr
    seq         c1, d.ingress_key2_d.match_fields[MATCH_ETHERNET_SRC_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ethernet_src_2, k.ethernet_2_srcAddr
    seq         c1, d.ingress_key2_d.match_fields[MATCH_ETHERNET_TYPE_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ethernet_type_2, k.ethernet_2_etherType
    seq         c1, d.ingress_key2_d.match_fields[MATCH_CUSTOMER_VLAN_ID_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ctag_2, \
                    k.{ctag_2_vid_sbit0_ebit3,ctag_2_vid_sbit4_ebit11}
    seq         c1, k.ipv4_2_valid, TRUE
    bcf         [c1], ingress_key2_ipv4
    seq         c1, k.ipv6_2_valid, TRUE
    bcf         [c1], ingress_key2_ipv6
    nop.!c1.e
    nop

ingress_key2_ipv4:
    seq         c1, d.ingress_key2_d.match_fields[MATCH_IP_SRC_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_src_2, k.ipv4_2_srcAddr
    seq         c1, d.ingress_key2_d.match_fields[MATCH_IP_DST_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_dst_2, k.ipv4_2_dstAddr
    seq         c1, d.ingress_key2_d.match_fields[MATCH_IP_DSCP_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_dscp_2, k.ipv4_2_diffserv
    seq         c1, d.ingress_key2_d.match_fields[MATCH_IP_PROTO_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_proto_2, k.ipv4_2_protocol
    seq.e       c1, d.ingress_key2_d.match_fields[MATCH_IP_TTL_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_ttl_2, k.ipv4_2_ttl

ingress_key2_ipv6:
    seq         c1, d.ingress_key2_d.match_fields[MATCH_IP_SRC_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_src_2[127:64], \
                    k.{ipv6_2_srcAddr_sbit0_ebit7...ipv6_2_srcAddr_sbit32_ebit63}
    phvwr.c1    p.flow_lkp_metadata_ip_src_2[63:0], \
                    k.{ipv6_2_srcAddr_sbit64_ebit95,ipv6_2_srcAddr_sbit96_ebit127}
    seq         c1, d.ingress_key2_d.match_fields[MATCH_IP_DST_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_dst_2[127:48], k.ipv6_2_dstAddr_sbit0_ebit79
    phvwr.c1    p.flow_lkp_metadata_ip_dst_2[47:0], k.ipv6_2_dstAddr_sbit80_ebit127
    seq         c1, d.ingress_key2_d.match_fields[MATCH_IP_DSCP_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_dscp_2, \
                    k.{ipv6_2_trafficClass_sbit0_ebit3,ipv6_2_trafficClass_sbit4_ebit7}
    seq         c1, d.ingress_key2_d.match_fields[MATCH_IP_PROTO_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_proto_2, k.ipv6_2_nextHdr
    seq.e       c1, d.ingress_key2_d.match_fields[MATCH_IP_TTL_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_ttl_2, k.ipv6_2_hopLimit

#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct ingress_key1_k k;
struct ingress_key1_d d;
struct phv_ p;

%%

ingress_key1:
    nop.!c1.e
    seq         c1, d.ingress_key1_d.match_fields[MATCH_ETHERNET_DST_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ethernet_dst_1, k.ethernet_1_dstAddr
    seq         c1, d.ingress_key1_d.match_fields[MATCH_ETHERNET_SRC_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ethernet_src_1, k.ethernet_1_srcAddr
    seq         c1, d.ingress_key1_d.match_fields[MATCH_ETHERNET_TYPE_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ethernet_type_1, k.ethernet_1_etherType
    seq         c1, d.ingress_key1_d.match_fields[MATCH_CUSTOMER_VLAN_ID_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ctag_1, \
                    k.{ctag_1_vid_sbit0_ebit3,ctag_1_vid_sbit4_ebit11}
    seq         c1, d.ingress_key1_d.match_fields[MATCH_SERVICE_VLAN_ID_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_stag_1, \
                    k.{stag_1_vid_sbit0_ebit3,stag_1_vid_sbit4_ebit11}
    seq         c1, k.ipv4_1_valid, TRUE
    bcf         [c1], ingress_key1_ipv4
    seq         c1, k.ipv6_1_valid, TRUE
    bcf         [c1], ingress_key1_ipv6
    nop.!c1.e
    nop

ingress_key1_ipv4:
    seq         c1, d.ingress_key1_d.match_fields[MATCH_IP_SRC_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_src_1, k.ipv4_1_srcAddr
    seq         c1, d.ingress_key1_d.match_fields[MATCH_IP_DST_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_dst_1, k.ipv4_1_dstAddr
    seq         c1, d.ingress_key1_d.match_fields[MATCH_IP_DSCP_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_dscp_1, k.ipv4_1_diffserv
    seq         c1, d.ingress_key1_d.match_fields[MATCH_IP_PROTO_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_proto_1, k.ipv4_1_protocol
    seq.e       c1, d.ingress_key1_d.match_fields[MATCH_IP_TTL_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_ttl_1, k.ipv4_1_ttl

ingress_key1_ipv6:
    seq         c1, d.ingress_key1_d.match_fields[MATCH_IP_SRC_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_src_1[127:64], \
                    k.{ipv6_1_srcAddr_sbit0_ebit7...ipv6_1_srcAddr_sbit32_ebit63}
    phvwr.c1    p.flow_lkp_metadata_ip_src_1[63:0], \
                    k.{ipv6_1_srcAddr_sbit64_ebit95,ipv6_1_srcAddr_sbit96_ebit127}
    seq         c1, d.ingress_key1_d.match_fields[MATCH_IP_DST_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_dst_1[127:48], k.ipv6_1_dstAddr_sbit0_ebit79
    phvwr.c1    p.flow_lkp_metadata_ip_dst_1[47:0], k.ipv6_1_dstAddr_sbit80_ebit127
    seq         c1, d.ingress_key1_d.match_fields[MATCH_IP_DSCP_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_dscp_1, \
                    k.{ipv6_1_trafficClass_sbit0_ebit3,ipv6_1_trafficClass_sbit4_ebit7}
    seq         c1, d.ingress_key1_d.match_fields[MATCH_IP_PROTO_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_proto_1, k.ipv6_1_nextHdr
    seq.e       c1, d.ingress_key1_d.match_fields[MATCH_IP_TTL_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_ttl_1, k.ipv6_1_hopLimit

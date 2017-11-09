#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct ingress_key3_k k;
struct ingress_key3_d d;
struct phv_ p;

%%

ingress_key3:
    nop.!c1.e
    seq         c1, d.ingress_key3_d.match_fields[MATCH_ETHERNET_DST_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ethernet_dst_3, k.ethernet_3_dstAddr
    seq         c1, d.ingress_key3_d.match_fields[MATCH_ETHERNET_SRC_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ethernet_src_3, k.ethernet_3_srcAddr
    seq         c1, d.ingress_key3_d.match_fields[MATCH_ETHERNET_TYPE_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ethernet_type_3, k.ethernet_3_etherType
    seq         c1, d.ingress_key3_d.match_fields[MATCH_CUSTOMER_VLAN_ID_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ctag_3, \
                    k.{ctag_3_vid_sbit0_ebit3,ctag_3_vid_sbit4_ebit11}
    seq         c1, d.ingress_key3_d.match_fields[MATCH_SERVICE_VLAN_ID_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_stag_3, \
                    k.{stag_3_vid_sbit0_ebit3,stag_3_vid_sbit4_ebit11}
    seq         c1, k.ipv4_3_valid, TRUE
    bcf         [c1], ingress_key3_ipv4
    seq         c1, k.ipv6_3_valid, TRUE
    bcf         [c1], ingress_key3_ipv6
    nop.!c1.e
    nop

ingress_key3_ipv4:
    seq         c1, d.ingress_key3_d.match_fields[MATCH_IP_SRC_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_src_3, k.ipv4_3_srcAddr
    seq         c1, d.ingress_key3_d.match_fields[MATCH_IP_DST_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_dst_3, k.ipv4_3_dstAddr
    seq         c1, d.ingress_key3_d.match_fields[MATCH_IP_DSCP_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_dscp_3, k.ipv4_3_diffserv
    seq         c1, d.ingress_key3_d.match_fields[MATCH_IP_PROTO_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_proto_3, k.ipv4_3_protocol
    seq.e       c1, d.ingress_key3_d.match_fields[MATCH_IP_TTL_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_ttl_3, k.ipv4_3_ttl

ingress_key3_ipv6:
    seq         c1, d.ingress_key3_d.match_fields[MATCH_IP_SRC_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_src_3, \
                    k.{ipv6_3_srcAddr_sbit0_ebit31,ipv6_3_srcAddr_sbit32_ebit127}
    seq         c1, d.ingress_key3_d.match_fields[MATCH_IP_DST_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_dst_3, \
                    k.{ipv6_3_dstAddr_sbit0_ebit31,ipv6_3_dstAddr_sbit32_ebit127}
    seq         c1, d.ingress_key3_d.match_fields[MATCH_IP_DSCP_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_dscp_3, \
                    k.{ipv6_3_trafficClass_sbit0_ebit3,ipv6_3_trafficClass_sbit4_ebit7}
    seq         c1, d.ingress_key3_d.match_fields[MATCH_IP_PROTO_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_proto_3, k.ipv6_3_nextHdr
    seq.e       c1, d.ingress_key3_d.match_fields[MATCH_IP_TTL_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_ip_ttl_3, k.ipv6_3_hopLimit

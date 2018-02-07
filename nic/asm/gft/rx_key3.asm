#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct rx_key3_k k;
struct rx_key3_d d;
struct phv_ p;

%%

rx_key3:
    nop.!c1.e
    crestore    [c4-c1], d.rx_key3_d.match_fields[3:0], 0xF
    phvwr.c1    p.flow_lkp_metadata_ethernet_dst_3, k.ethernet_3_dstAddr
    phvwr.c2    p.flow_lkp_metadata_ethernet_src_3, k.ethernet_3_srcAddr
    phvwr.c3    p.flow_lkp_metadata_ethernet_type_3, k.ethernet_3_etherType
    phvwr.c4    p.flow_lkp_metadata_ctag_3, \
                    k.{ctag_3_vid_sbit0_ebit3,ctag_3_vid_sbit4_ebit11}
    bbeq        k.ipv4_3_valid, TRUE, rx_key3_ipv4
    crestore    [c5-c1], d.rx_key3_d.match_fields[8:4], 0x1F
    bbeq        k.ipv6_3_valid, TRUE, rx_key3_ipv6
    nop
    nop.e
    nop

rx_key3_ipv4:
    phvwr.c1    p.flow_lkp_metadata_ip_src_3, k.ipv4_3_srcAddr
    phvwr.c2    p.flow_lkp_metadata_ip_dst_3, k.ipv4_3_dstAddr
    phvwr.c3    p.flow_lkp_metadata_ip_dscp_3, k.ipv4_3_diffserv
    phvwr.c4    p.flow_lkp_metadata_ip_proto_3, k.ipv4_3_protocol
    nop.e
    phvwr.c5    p.flow_lkp_metadata_ip_ttl_3, k.ipv4_3_ttl

rx_key3_ipv6:
    phvwr.c1    p.flow_lkp_metadata_ip_src_3, \
                    k.{ipv6_3_srcAddr_sbit0_ebit7...ipv6_3_srcAddr_sbit96_ebit127}
    phvwr.c2    p.flow_lkp_metadata_ip_dst_3[127:40], k.ipv6_3_dstAddr_sbit0_ebit87
    phvwr.c2    p.flow_lkp_metadata_ip_dst_3[39:0], k.ipv6_3_dstAddr_sbit88_ebit127
    phvwr.c3    p.flow_lkp_metadata_ip_dscp_3, \
                    k.{ipv6_3_trafficClass_sbit0_ebit3,ipv6_3_trafficClass_sbit4_ebit7}
    phvwr.c4    p.flow_lkp_metadata_ip_proto_3, k.ipv6_3_nextHdr
    nop.e
    phvwr.c5    p.flow_lkp_metadata_ip_ttl_3, k.ipv6_3_hopLimit

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rx_key3_error:
    nop.e
    nop

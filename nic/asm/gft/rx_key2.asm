#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct rx_key2_k k;
struct rx_key2_d d;
struct phv_ p;

%%

rx_key2:
    nop.!c1.e
    seq         c1, d.rx_key2_d.match_fields, r0
    nop.c1.e
    crestore    [c4-c1], d.rx_key2_d.match_fields[3:0], 0xF
    phvwr.c1    p.flow_lkp_metadata_ethernet_dst_2, k.ethernet_2_dstAddr
    add         r7, r0, r0
    or.c2       r7, r7, k.ethernet_2_srcAddr, 16
    or.c3       r7, r7, k.ethernet_2_etherType
    phvwr       p.{flow_lkp_metadata_ethernet_src_2, \
                   flow_lkp_metadata_ethernet_type_2}, r7
    phvwr.c4    p.flow_lkp_metadata_ctag_2, \
                    k.{ctag_2_vid_sbit0_ebit3,ctag_2_vid_sbit4_ebit11}
    bbeq        k.ipv4_2_valid, TRUE, rx_key2_ipv4
    crestore    [c5-c1], d.rx_key2_d.match_fields[8:4], 0x1F
    bbeq        k.ipv6_2_valid, TRUE, rx_key2_ipv6
    nop
    nop.e
    nop

rx_key2_ipv4:
    phvwr.c1    p.flow_lkp_metadata_ip_src_2, k.ipv4_2_srcAddr
    phvwr.c2    p.flow_lkp_metadata_ip_dst_2, k.ipv4_2_dstAddr
    add         r7, r0, r0
    or.c3       r7, r7, k.ipv4_2_diffserv, 16
    or.c4       r7, r7, k.ipv4_2_protocol, 8
    or.c5       r7, r7, k.ipv4_2_ttl
    phvwr.f.e   p.{flow_lkp_metadata_ip_dscp_2,flow_lkp_metadata_ip_proto_2, \
                   flow_lkp_metadata_ip_ttl_2}, r7
    nop

rx_key2_ipv6:
    phvwr.c1    p.flow_lkp_metadata_ip_src_2[127:64], \
                    k.{ipv6_2_srcAddr_sbit0_ebit7...ipv6_2_srcAddr_sbit32_ebit63}
    phvwr.c1    p.flow_lkp_metadata_ip_src_2[63:0], \
                    k.{ipv6_2_srcAddr_sbit64_ebit95,ipv6_2_srcAddr_sbit96_ebit127}
    phvwr.c2    p.flow_lkp_metadata_ip_dst_2[127:40], k.ipv6_2_dstAddr_sbit0_ebit87
    phvwr.c2    p.flow_lkp_metadata_ip_dst_2[39:0], k.ipv6_2_dstAddr_sbit88_ebit127
    add         r7, r0, r0
    or.c3       r7, r7, k.{ipv6_2_trafficClass_sbit0_ebit3,\
                           ipv6_2_trafficClass_sbit4_ebit7}, 16
    or.c4       r7, r7, k.ipv6_2_nextHdr, 8
    or.c5       r7, r7, k.ipv6_2_hopLimit
    phvwr.f.e   p.{flow_lkp_metadata_ip_dscp_2,flow_lkp_metadata_ip_proto_2, \
                   flow_lkp_metadata_ip_ttl_2}, r7
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rx_key2_error:
    nop.e
    nop

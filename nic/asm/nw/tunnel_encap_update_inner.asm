#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct tunnel_encap_update_inner_k k;
struct phv_                        p;

%%

nop:
  nop.e
  nop

.align
encap_inner_ipv4_udp_rewrite:
  phvwr       p.{inner_ipv4_version...inner_ipv4_diffserv}, k.{ipv4_version...ipv4_diffserv}
  phvwr       p.{inner_ipv4_totalLen...inner_ipv4_srcAddr}, k.{ipv4_totalLen...ipv4_srcAddr}
  // TODO: Start
  // TTL and protocol had to be copied over individually since it is not being copied over
  // using the ellipses above even though they are contiguous in the K vector
  phvwr       p.inner_ipv4_diffserv, k.ipv4_diffserv
  phvwr       p.inner_ipv4_ttl, k.ipv4_ttl
  phvwr       p.inner_ipv4_protocol, k.ipv4_protocol
  // TODO: End
  phvwr       p.inner_ipv4_dstAddr, k.ipv4_dstAddr
  phvwr       p.{inner_udp_srcPort...inner_udp_checksum}, k.{udp_srcPort...udp_checksum}
  phvwr       p.tunnel_metadata_inner_ip_proto, IP_PROTO_UDP
  phvwr       p.udp_valid, FALSE
  phvwr       p.ipv4_valid, FALSE
  phvwr.e     p.inner_udp_valid, TRUE
  phvwr       p.inner_ipv4_valid, TRUE

.align
encap_inner_ipv4_tcp_rewrite:
encap_inner_ipv4_icmp_rewrite:
encap_inner_ipv4_unknown_rewrite:
  phvwr       p.{inner_ipv4_version...inner_ipv4_diffserv}, k.{ipv4_version...ipv4_diffserv}
  phvwr       p.{inner_ipv4_totalLen...inner_ipv4_srcAddr}, k.{ipv4_totalLen...ipv4_srcAddr}
  // TODO: Start
  // TTL and protocol had to be copied over individually since it is not being copied over
  // using the ellipses above even though they are contiguous in the K vector
  phvwr       p.inner_ipv4_ttl, k.ipv4_ttl
  phvwr       p.inner_ipv4_protocol, k.ipv4_protocol
  // TODO: End
  phvwr       p.inner_ipv4_dstAddr, k.ipv4_dstAddr
  phvwr       p.tunnel_metadata_inner_ip_proto, IP_PROTO_IPV4
  phvwr.e     p.ipv4_valid, FALSE
  phvwr       p.inner_ipv4_valid, TRUE


.align
encap_inner_ipv6_udp_rewrite:
//  phvwr       p.{inner_ipv6_version...inner_ipv6_flowLabel}, k.{ipv6_version,\
//                                                                 ipv6_trafficClass_sbit0_ebit3,\
//                                                                 ipv6_trafficClass_sbit4_ebit7,\
//                                                                 ipv6_flowLabel_sbit0_ebit3,\
//                                                                 ipv6_flowLabel_sbit4_ebit19}
//  phvwr       p.{inner_ipv6_payloadLen...inner_ipv6_dstAddr[127:120]}, \
//                  k.{ipv6_nextHdr...ipv6_dstAddr_sbit0_ebit7}
//  phvwr       p.inner_ipv6_dstAddr[119:0], k.ipv6_dstAddr_sbit8_ebit127
//  phvwr       p.{inner_udp_srcPort...inner_udp_checksum}, k.{udp_srcPort...udp_checksum}
  // TODO: Start
  // Ellipses usage fixed in latest CAPSIM version. Workaround till we get the latest CAPSIM
  phvwr         p.inner_ipv6_version, k.ipv6_version
  phvwr         p.inner_ipv6_trafficClass[7:4], k.ipv6_trafficClass_sbit0_ebit3
  phvwr         p.inner_ipv6_trafficClass[3:0], k.ipv6_trafficClass_sbit4_ebit7
  phvwr         p.inner_ipv6_flowLabel[19:16], k.ipv6_flowLabel_sbit0_ebit3
  phvwr         p.inner_ipv6_flowLabel[15:0], k.ipv6_flowLabel_sbit4_ebit19
  phvwr         p.inner_ipv6_payloadLen, k.ipv6_payloadLen
  phvwr         p.inner_ipv6_nextHdr, k.ipv6_nextHdr
  phvwr         p.inner_ipv6_hopLimit, k.ipv6_hopLimit

  phvwr         p.inner_ipv6_srcAddr[127:120], k.ipv6_srcAddr_sbit0_ebit7
  phvwr         p.inner_ipv6_srcAddr[119:112], k.ipv6_srcAddr_sbit8_ebit15
  phvwr         p.inner_ipv6_srcAddr[111:96], k.ipv6_srcAddr_sbit16_ebit31
  phvwr         p.inner_ipv6_srcAddr[95:64], k.ipv6_srcAddr_sbit32_ebit63
  phvwr         p.inner_ipv6_srcAddr[63:32], k.ipv6_srcAddr_sbit64_ebit95
  phvwr         p.inner_ipv6_srcAddr[31:0], k.ipv6_srcAddr_sbit96_ebit127
  phvwr         p.inner_ipv6_dstAddr[127:120], k.ipv6_dstAddr_sbit0_ebit7
  phvwr         p.inner_ipv6_dstAddr[119:0], k.ipv6_dstAddr_sbit8_ebit127

  phvwr         p.inner_udp_srcPort, k.udp_srcPort
  phvwr         p.inner_udp_dstPort, k.udp_dstPort
  phvwr         p.inner_udp_len, k.udp_len
  phvwr         p.inner_udp_checksum, k.udp_checksum
  // TODO: End

  phvwr       p.tunnel_metadata_inner_ip_proto, IP_PROTO_IPV6
  phvwr       p.udp_valid, FALSE
  phvwr       p.ipv6_valid, FALSE
  phvwr.e     p.inner_udp_valid, TRUE
  phvwr       p.inner_ipv6_valid, TRUE

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
encap_inner_ipv6_tcp_rewrite:
encap_inner_ipv6_icmp_rewrite:
encap_inner_ipv6_unknown_rewrite:
//  phvwr       p.{inner_ipv6_version...inner_ipv6_flowLabel}, k.{ipv6_version,\
//                                                                 ipv6_trafficClass_sbit0_ebit3,\
//                                                                 ipv6_trafficClass_sbit4_ebit7,\
//                                                                 ipv6_flowLabel_sbit0_ebit3,\
//                                                                 ipv6_flowLabel_sbit4_ebit19}
//  phvwr       p.{inner_ipv6_payloadLen...inner_ipv6_dstAddr[127:120]}, \
//                  k.{ipv6_nextHdr...ipv6_dstAddr_sbit0_ebit7}
//  phvwr       p.inner_ipv6_dstAddr[119:0], k.ipv6_dstAddr_sbit8_ebit127
  // TODO: Start
  // Ellipses usage fixed in latest CAPSIM version. Workaround till we get the latest CAPSIM
  phvwr         p.inner_ipv6_version, k.ipv6_version
  phvwr         p.inner_ipv6_trafficClass[7:4], k.ipv6_trafficClass_sbit0_ebit3
  phvwr         p.inner_ipv6_trafficClass[3:0], k.ipv6_trafficClass_sbit4_ebit7
  phvwr         p.inner_ipv6_flowLabel[19:16], k.ipv6_flowLabel_sbit0_ebit3
  phvwr         p.inner_ipv6_flowLabel[15:0], k.ipv6_flowLabel_sbit4_ebit19
  phvwr         p.inner_ipv6_payloadLen, k.ipv6_payloadLen
  phvwr         p.inner_ipv6_nextHdr, k.ipv6_nextHdr
  phvwr         p.inner_ipv6_hopLimit, k.ipv6_hopLimit

  phvwr         p.inner_ipv6_srcAddr[127:120], k.ipv6_srcAddr_sbit0_ebit7
  phvwr         p.inner_ipv6_srcAddr[119:112], k.ipv6_srcAddr_sbit8_ebit15
  phvwr         p.inner_ipv6_srcAddr[111:96], k.ipv6_srcAddr_sbit16_ebit31
  phvwr         p.inner_ipv6_srcAddr[95:64], k.ipv6_srcAddr_sbit32_ebit63
  phvwr         p.inner_ipv6_srcAddr[63:32], k.ipv6_srcAddr_sbit64_ebit95
  phvwr         p.inner_ipv6_srcAddr[31:0], k.ipv6_srcAddr_sbit96_ebit127
  phvwr         p.inner_ipv6_dstAddr[127:120], k.ipv6_dstAddr_sbit0_ebit7
  phvwr         p.inner_ipv6_dstAddr[119:0], k.ipv6_dstAddr_sbit8_ebit127

  // TODO: End
  phvwr       p.tunnel_metadata_inner_ip_proto, IP_PROTO_IPV6
  phvwr.e     p.ipv6_valid, FALSE
  phvwr       p.inner_ipv6_valid, TRUE

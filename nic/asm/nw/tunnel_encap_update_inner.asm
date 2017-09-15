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
  phvwr       p.{inner_ipv4_totalLen...inner_ipv4_dstAddr}, k.{ipv4_totalLen...ipv4_dstAddr}
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
  phvwr       p.{inner_ipv4_totalLen...inner_ipv4_dstAddr}, k.{ipv4_totalLen...ipv4_dstAddr}
  phvwr       p.tunnel_metadata_inner_ip_proto, IP_PROTO_IPV4
  phvwr.e     p.ipv4_valid, FALSE
  phvwr       p.inner_ipv4_valid, TRUE


.align
encap_inner_ipv6_udp_rewrite:
  phvwr       p.{inner_ipv6_version...inner_ipv6_flowLabel[19:0]}, k.{ipv6_version...ipv6_flowLabel_sbit4_ebit19}
  phvwr       p.{inner_ipv6_payloadLen...inner_ipv6_dstAddr[127:120]}, k.{ipv6_payloadLen...ipv6_dstAddr_sbit0_ebit7}
  phvwr       p.inner_ipv6_dstAddr[119:0], k.ipv6_dstAddr_sbit8_ebit127
  phvwr       p.{inner_udp_srcPort...inner_udp_checksum}, k.{udp_srcPort...udp_checksum}
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
  phvwr       p.{inner_ipv6_version...inner_ipv6_flowLabel[19:0]}, k.{ipv6_version...ipv6_flowLabel_sbit4_ebit19}
  phvwr       p.{inner_ipv6_payloadLen...inner_ipv6_dstAddr[127:120]}, k.{ipv6_payloadLen...ipv6_dstAddr_sbit0_ebit7}
  phvwr       p.inner_ipv6_dstAddr[119:0], k.ipv6_dstAddr_sbit8_ebit127
  phvwr       p.tunnel_metadata_inner_ip_proto, IP_PROTO_IPV6
  phvwr.e     p.ipv6_valid, FALSE
  phvwr       p.inner_ipv6_valid, TRUE

#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct tunnel_decap_copy_inner_k k;
struct tunnel_decap_copy_inner_d d;
struct phv_                      p;

%%

nop:
  nop.e
  nop

.align
copy_inner_ipv4_udp:
  phvwr       p.{ipv4_version...ipv4_diffserv}, k.{inner_ipv4_version...inner_ipv4_diffserv}
  phvwr       p.{ipv4_totalLen...ipv4_srcAddr}, k.{inner_ipv4_totalLen...inner_ipv4_srcAddr}
  phvwr       p.ipv4_dstAddr, k.inner_ipv4_dstAddr
  phvwr       p.{udp_srcPort...udp_checksum}, k.{inner_udp_srcPort...inner_udp_checksum}
  phvwr       p.ipv4_valid, TRUE
  phvwr       p.udp_valid, TRUE
  phvwr.e     p.inner_ipv4_valid, FALSE
  phvwr       p.inner_udp_valid, FALSE

.align
copy_inner_ipv4_other:
  phvwr       p.{ipv4_version...ipv4_diffserv}, k.{inner_ipv4_version...inner_ipv4_diffserv}
  phvwr       p.{ipv4_totalLen...ipv4_srcAddr}, k.{inner_ipv4_totalLen...inner_ipv4_srcAddr}
  phvwr       p.ipv4_dstAddr, k.inner_ipv4_dstAddr
  phvwr       p.ipv4_valid, TRUE
  phvwr.e     p.inner_ipv4_valid, FALSE
  phvwr       p.udp_valid, FALSE

.align
copy_inner_ipv6_udp:
  phvwr       p.{ipv6_version...ipv6_srcAddr[79:0]}, k.{inner_ipv6_version...inner_ipv6_srcAddr_sbit48_ebit79}
  phvwr       p.{ipv6_srcAddr[127:80]...ipv6_dstAddr}, k.{inner_ipv6_srcAddr_sbit80_ebit111...inner_ipv6_srcAddr_sbit112_ebit127}
  phvwr       p.{udp_srcPort...udp_checksum}, k.{inner_udp_srcPort...inner_udp_checksum}
  phvwr       p.ipv6_valid, TRUE
  phvwr       p.udp_valid, TRUE
  phvwr.e     p.inner_ipv6_valid, FALSE
  phvwr       p.inner_udp_valid, FALSE

.align
copy_inner_ipv6_other:
  phvwr       p.{ipv6_version...ipv6_srcAddr[79:0]}, k.{inner_ipv6_version...inner_ipv6_srcAddr_sbit48_ebit79}
  phvwr       p.{ipv6_srcAddr[127:80]...ipv6_dstAddr}, k.{inner_ipv6_srcAddr_sbit80_ebit111...inner_ipv6_srcAddr_sbit112_ebit127}
  phvwr       p.ipv6_valid, TRUE
  phvwr.e     p.inner_ipv6_valid, FALSE
  phvwr       p.udp_valid, FALSE

.align
copy_inner_eth_ipv4_udp:
  phvwr       p.ethernet_dstAddr, k.inner_ethernet_dstAddr
  phvwr       p.{ethernet_srcAddr, ethernet_etherType}, k.{inner_ethernet_srcAddr, inner_ethernet_etherType}
  phvwr       p.{ipv4_version...ipv4_diffserv}, k.{inner_ipv4_version...inner_ipv4_diffserv}
  phvwr       p.{ipv4_totalLen...ipv4_srcAddr}, k.{inner_ipv4_totalLen...inner_ipv4_srcAddr}
  phvwr       p.ipv4_dstAddr, k.inner_ipv4_dstAddr
  phvwr       p.{udp_srcPort...udp_checksum}, k.{inner_udp_srcPort...inner_udp_checksum}
  phvwr       p.ethernet_valid, TRUE
  phvwr       p.ipv4_valid, TRUE
  phvwr       p.udp_valid, TRUE
  phvwr       p.inner_ethernet_valid, FALSE
  phvwr       p.vlan_tag_valid, FALSE
  phvwr.e     p.inner_ipv4_valid, FALSE
  phvwr       p.inner_udp_valid, FALSE

.align
copy_inner_eth_ipv4_other:
  phvwr       p.ethernet_dstAddr, k.inner_ethernet_dstAddr
  phvwr       p.{ethernet_srcAddr, ethernet_etherType}, k.{inner_ethernet_srcAddr, inner_ethernet_etherType}
  phvwr       p.{ipv4_version...ipv4_diffserv}, k.{inner_ipv4_version...inner_ipv4_diffserv}
  phvwr       p.{ipv4_totalLen...ipv4_srcAddr}, k.{inner_ipv4_totalLen...inner_ipv4_srcAddr}
  phvwr       p.ipv4_dstAddr, k.inner_ipv4_dstAddr
  phvwr       p.ethernet_valid, TRUE
  phvwr       p.ipv4_valid, TRUE
  phvwr       p.inner_ethernet_valid, FALSE
  phvwr       p.vlan_tag_valid, FALSE
  phvwr.e     p.inner_ipv4_valid, FALSE
  phvwr       p.udp_valid, FALSE

.align
copy_inner_eth_ipv6_udp:
  phvwr       p.ethernet_dstAddr, k.inner_ethernet_dstAddr
  phvwr       p.{ethernet_srcAddr, ethernet_etherType}, k.{inner_ethernet_srcAddr, inner_ethernet_etherType}
  phvwr       p.{ipv6_version...ipv6_srcAddr[79:0]}, k.{inner_ipv6_version...inner_ipv6_srcAddr_sbit48_ebit79}
  phvwr       p.{ipv6_srcAddr[127:80]...ipv6_dstAddr}, k.{inner_ipv6_srcAddr_sbit80_ebit111...inner_ipv6_srcAddr_sbit112_ebit127}
  phvwr       p.{udp_srcPort...udp_checksum}, k.{inner_udp_srcPort...inner_udp_checksum}
  phvwr       p.ethernet_valid, TRUE
  phvwr       p.ipv6_valid, TRUE
  phvwr       p.udp_valid, TRUE
  phvwr       p.inner_ethernet_valid, FALSE
  phvwr       p.vlan_tag_valid, FALSE
  phvwr.e     p.inner_ipv6_valid, FALSE
  phvwr       p.inner_udp_valid, FALSE

.align
copy_inner_eth_ipv6_other:
  phvwr       p.ethernet_dstAddr, k.inner_ethernet_dstAddr
  phvwr       p.{ethernet_srcAddr, ethernet_etherType}, k.{inner_ethernet_srcAddr, inner_ethernet_etherType}
  phvwr       p.{ipv6_version...ipv6_srcAddr[79:0]}, k.{inner_ipv6_version...inner_ipv6_srcAddr_sbit48_ebit79}
  phvwr       p.{ipv6_srcAddr[127:80]...ipv6_dstAddr}, k.{inner_ipv6_srcAddr_sbit80_ebit111...inner_ipv6_srcAddr_sbit112_ebit127}
  phvwr       p.ethernet_valid, TRUE
  phvwr       p.ipv6_valid, TRUE
  phvwr       p.inner_ethernet_valid, FALSE
  phvwr       p.vlan_tag_valid, FALSE
  phvwr.e     p.inner_ipv6_valid, FALSE
  phvwr       p.udp_valid, FALSE

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
copy_inner_eth_non_ip:
  phvwr       p.ethernet_dstAddr, k.inner_ethernet_dstAddr
  phvwr       p.{ethernet_srcAddr, ethernet_etherType}, k.{inner_ethernet_srcAddr, inner_ethernet_etherType}
  phvwr       p.ethernet_valid, TRUE
  phvwr       p.inner_ethernet_valid, FALSE
  phvwr.e     p.vlan_tag_valid, FALSE
  phvwr       p.udp_valid, FALSE

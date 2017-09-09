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
  //phvwr       p.{ipv6_version...ipv6_srcAddr[79:0]}, k.{inner_ipv6_version...inner_ipv6_srcAddr_sbit48_ebit79}
  //phvwr       p.{ipv6_srcAddr[127:80]...ipv6_dstAddr}, k.{inner_ipv6_srcAddr_sbit80_ebit111...inner_ipv6_srcAddr_sbit112_ebit127}
  //phvwr       p.{udp_srcPort...udp_checksum}, k.{inner_udp_srcPort...inner_udp_checksum}
  // TODO: Start
  // Ellipses usage fixed in latest CAPSIM version. Workaround till we get the latest CAPSIM
  phvwr         p.ipv6_version, k.inner_ipv6_version
  phvwr         p.ipv6_trafficClass[7:4], k.inner_ipv6_trafficClass_sbit0_ebit3
  phvwr         p.ipv6_trafficClass[3:0], k.inner_ipv6_trafficClass_sbit4_ebit7
  phvwr         p.ipv6_flowLabel[19:16], k.inner_ipv6_flowLabel_sbit0_ebit3
  phvwr         p.ipv6_flowLabel[15:0], k.inner_ipv6_flowLabel_sbit4_ebit19
  phvwr         p.ipv6_payloadLen, k.inner_ipv6_payloadLen
  phvwr         p.ipv6_nextHdr, k.inner_ipv6_nextHdr
  phvwr         p.ipv6_hopLimit, k.inner_ipv6_hopLimit

  //phvwr         p.ipv6_srcAddr, k.inner_ipv6_srcAddr
  phvwr         p.ipv6_srcAddr[127:120], k.inner_ipv6_srcAddr_sbit0_ebit7
  phvwr         p.ipv6_srcAddr[119:112], k.inner_ipv6_srcAddr_sbit8_ebit15
  phvwr         p.ipv6_srcAddr[111:104], k.inner_ipv6_srcAddr_sbit16_ebit23
  phvwr         p.ipv6_srcAddr[103:96], k.inner_ipv6_srcAddr_sbit24_ebit31
  phvwr         p.ipv6_srcAddr[95:80], k.inner_ipv6_srcAddr_sbit32_ebit47
  phvwr         p.ipv6_srcAddr[79:48], k.inner_ipv6_srcAddr_sbit48_ebit79
  phvwr         p.ipv6_srcAddr[47:16], k.inner_ipv6_srcAddr_sbit80_ebit111
  phvwr         p.ipv6_srcAddr[15:0], k.inner_ipv6_srcAddr_sbit112_ebit127
  phvwr         p.ipv6_dstAddr, k.inner_ipv6_dstAddr

  phvwr         p.udp_srcPort, k.inner_udp_srcPort
  phvwr         p.udp_dstPort, k.inner_udp_dstPort
  phvwr         p.udp_len, k.inner_udp_len
  phvwr         p.udp_checksum, k.inner_udp_checksum
  // TODO: End

  phvwr       p.ipv6_valid, TRUE
  phvwr       p.udp_valid, TRUE
  phvwr.e     p.inner_ipv6_valid, FALSE
  phvwr       p.inner_udp_valid, FALSE

.align
copy_inner_ipv6_other:
  //phvwr       p.{ipv6_version...ipv6_srcAddr[79:0]}, k.{inner_ipv6_version...inner_ipv6_srcAddr_sbit48_ebit79}
  //phvwr       p.{ipv6_srcAddr[127:80]...ipv6_dstAddr}, k.{inner_ipv6_srcAddr_sbit80_ebit111...inner_ipv6_srcAddr_sbit112_ebit127}
  // TODO: Start
  // Ellipses usage fixed in latest CAPSIM version. Workaround till we get the latest CAPSIM
  phvwr         p.ipv6_version, k.inner_ipv6_version
  phvwr         p.ipv6_trafficClass[7:4], k.inner_ipv6_trafficClass_sbit0_ebit3
  phvwr         p.ipv6_trafficClass[3:0], k.inner_ipv6_trafficClass_sbit4_ebit7
  phvwr         p.ipv6_flowLabel[19:16], k.inner_ipv6_flowLabel_sbit0_ebit3
  phvwr         p.ipv6_flowLabel[15:0], k.inner_ipv6_flowLabel_sbit4_ebit19
  phvwr         p.ipv6_payloadLen, k.inner_ipv6_payloadLen
  phvwr         p.ipv6_nextHdr, k.inner_ipv6_nextHdr
  phvwr         p.ipv6_hopLimit, k.inner_ipv6_hopLimit

  phvwr         p.ipv6_srcAddr[127:120], k.inner_ipv6_srcAddr_sbit0_ebit7
  phvwr         p.ipv6_srcAddr[119:112], k.inner_ipv6_srcAddr_sbit8_ebit15
  phvwr         p.ipv6_srcAddr[111:104], k.inner_ipv6_srcAddr_sbit16_ebit23
  phvwr         p.ipv6_srcAddr[103:96], k.inner_ipv6_srcAddr_sbit24_ebit31
  phvwr         p.ipv6_srcAddr[95:80], k.inner_ipv6_srcAddr_sbit32_ebit47
  phvwr         p.ipv6_srcAddr[79:48], k.inner_ipv6_srcAddr_sbit48_ebit79
  phvwr         p.ipv6_srcAddr[47:16], k.inner_ipv6_srcAddr_sbit80_ebit111
  phvwr         p.ipv6_srcAddr[15:0], k.inner_ipv6_srcAddr_sbit112_ebit127
  phvwr         p.ipv6_dstAddr, k.inner_ipv6_dstAddr
  // TODO: End

  phvwr       p.ipv6_valid, TRUE
  phvwr.e     p.inner_ipv6_valid, FALSE
  phvwr       p.udp_valid, FALSE

.align
copy_inner_eth_ipv4_udp:
  phvwr       p.ethernet_dstAddr, k.inner_ethernet_dstAddr
  phvwr       p.{ethernet_srcAddr, ethernet_etherType}, k.{inner_ethernet_srcAddr, inner_ethernet_etherType}
  phvwr       p.{ipv4_version...ipv4_diffserv}, k.{inner_ipv4_version...inner_ipv4_diffserv}
  phvwr       p.{ipv4_totalLen...ipv4_srcAddr}, k.{inner_ipv4_totalLen...inner_ipv4_srcAddr}
  // TODO: Start
  // TTL and protocol had to be copied over individually since it is not being copied over
  // using the ellipses above even though they are contiguous in the K vector
  phvwr       p.ipv4_ttl, k.inner_ipv4_ttl
  phvwr       p.ipv4_protocol, k.inner_ipv4_protocol
  // TODO: End
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
  // TODO: Start
  // TTL and protocol had to be copied over individually since it is not being copied over
  // using the ellipses above even though they are contiguous in the K vector
  phvwr       p.ipv4_ttl, k.inner_ipv4_ttl
  phvwr       p.ipv4_protocol, k.inner_ipv4_protocol
  // TODO: End
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

  //phvwr       p.{ipv6_version...ipv6_srcAddr[79:0]}, k.{inner_ipv6_version...inner_ipv6_srcAddr_sbit48_ebit79}
  //phvwr       p.{ipv6_srcAddr[127:80]...ipv6_dstAddr}, k.{inner_ipv6_srcAddr_sbit80_ebit111...inner_ipv6_srcAddr_sbit112_ebit127}
  //phvwr       p.{udp_srcPort...udp_checksum}, k.{inner_udp_srcPort...inner_udp_checksum}

  // TODO: Start
  // Ellipses usage fixed in latest CAPSIM version. Workaround till we get the latest CAPSIM
  phvwr         p.ipv6_version, k.inner_ipv6_version
  phvwr         p.ipv6_trafficClass[7:4], k.inner_ipv6_trafficClass_sbit0_ebit3
  phvwr         p.ipv6_trafficClass[3:0], k.inner_ipv6_trafficClass_sbit4_ebit7
  phvwr         p.ipv6_flowLabel[19:16], k.inner_ipv6_flowLabel_sbit0_ebit3
  phvwr         p.ipv6_flowLabel[15:0], k.inner_ipv6_flowLabel_sbit4_ebit19
  phvwr         p.ipv6_payloadLen, k.inner_ipv6_payloadLen
  phvwr         p.ipv6_nextHdr, k.inner_ipv6_nextHdr
  phvwr         p.ipv6_hopLimit, k.inner_ipv6_hopLimit

  //phvwr         p.ipv6_srcAddr, k.inner_ipv6_srcAddr
  phvwr         p.ipv6_srcAddr[127:120], k.inner_ipv6_srcAddr_sbit0_ebit7
  phvwr         p.ipv6_srcAddr[119:112], k.inner_ipv6_srcAddr_sbit8_ebit15
  phvwr         p.ipv6_srcAddr[111:104], k.inner_ipv6_srcAddr_sbit16_ebit23
  phvwr         p.ipv6_srcAddr[103:96], k.inner_ipv6_srcAddr_sbit24_ebit31
  phvwr         p.ipv6_srcAddr[95:80], k.inner_ipv6_srcAddr_sbit32_ebit47
  phvwr         p.ipv6_srcAddr[79:48], k.inner_ipv6_srcAddr_sbit48_ebit79
  phvwr         p.ipv6_srcAddr[47:16], k.inner_ipv6_srcAddr_sbit80_ebit111
  phvwr         p.ipv6_srcAddr[15:0], k.inner_ipv6_srcAddr_sbit112_ebit127
  phvwr         p.ipv6_dstAddr, k.inner_ipv6_dstAddr

  phvwr         p.udp_srcPort, k.inner_udp_srcPort
  phvwr         p.udp_dstPort, k.inner_udp_dstPort
  phvwr         p.udp_len, k.inner_udp_len
  phvwr         p.udp_checksum, k.inner_udp_checksum
  // TODO: End

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
  //phvwr       p.{ipv6_version...ipv6_srcAddr[79:0]}, k.{inner_ipv6_version...inner_ipv6_srcAddr_sbit48_ebit79}
  //phvwr       p.{ipv6_srcAddr[127:80]...ipv6_dstAddr}, k.{inner_ipv6_srcAddr_sbit80_ebit111...inner_ipv6_srcAddr_sbit112_ebit127}
  
  // TODO: Start
  // Ellipses usage fixed in latest CAPSIM version. Workaround till we get the latest CAPSIM
  phvwr         p.ipv6_version, k.inner_ipv6_version
  phvwr         p.ipv6_trafficClass[7:4], k.inner_ipv6_trafficClass_sbit0_ebit3
  phvwr         p.ipv6_trafficClass[3:0], k.inner_ipv6_trafficClass_sbit4_ebit7
  phvwr         p.ipv6_flowLabel[19:16], k.inner_ipv6_flowLabel_sbit0_ebit3
  phvwr         p.ipv6_flowLabel[15:0], k.inner_ipv6_flowLabel_sbit4_ebit19
  phvwr         p.ipv6_payloadLen, k.inner_ipv6_payloadLen
  phvwr         p.ipv6_nextHdr, k.inner_ipv6_nextHdr
  phvwr         p.ipv6_hopLimit, k.inner_ipv6_hopLimit

  //phvwr         p.ipv6_srcAddr, k.inner_ipv6_srcAddr
  phvwr         p.ipv6_srcAddr[127:120], k.inner_ipv6_srcAddr_sbit0_ebit7
  phvwr         p.ipv6_srcAddr[119:112], k.inner_ipv6_srcAddr_sbit8_ebit15
  phvwr         p.ipv6_srcAddr[111:104], k.inner_ipv6_srcAddr_sbit16_ebit23
  phvwr         p.ipv6_srcAddr[103:96], k.inner_ipv6_srcAddr_sbit24_ebit31
  phvwr         p.ipv6_srcAddr[95:80], k.inner_ipv6_srcAddr_sbit32_ebit47
  phvwr         p.ipv6_srcAddr[79:48], k.inner_ipv6_srcAddr_sbit48_ebit79
  phvwr         p.ipv6_srcAddr[47:16], k.inner_ipv6_srcAddr_sbit80_ebit111
  phvwr         p.ipv6_srcAddr[15:0], k.inner_ipv6_srcAddr_sbit112_ebit127
  phvwr         p.ipv6_dstAddr, k.inner_ipv6_dstAddr
  // TODO: End

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
  phvwr       p.ipv4_valid, FALSE
  phvwr       p.ipv6_valid, FALSE
  phvwr.e     p.vlan_tag_valid, FALSE
  phvwr       p.udp_valid, FALSE

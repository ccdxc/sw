/******************************************************************************/
TABLE : tunnel_decap_copy_inner
/******************************************************************************/
Format 1 (IPv4):
struct k {
  phv.inner_ethernet_valid : 1;
  phv.inner_udp_valid : 1;

  // 112 bits
  inner_ethernet_dstAddr : 48;
  inner_ethernet_srcAddr : 48;
  inner_ethernet_etherType : 16;

  // 160 bits
  inner_ipv4_version : 4;
  inner_ipv4_ihl : 4;
  inner_ipv4_diffserv : 8;
  inner_ipv4_totalLen : 16;
  inner_ipv4_identification : 16;
  inner_ipv4_flags : 3;
  inner_ipv4_fragOffset : 13;
  inner_ipv4_ttl : 8;
  inner_ipv4_protocol : 8;
  inner_ipv4_hdrChecksum : 16;
  inner_ipv4_srcAddr : 32;
  inner_ipv4_dstAddr: 32;

  // 64 bits
  inner_udp_srcPort : 16;
  inner_udp_dstPort : 16;
  inner_udp_len : 16;
  inner_udp_checksum : 16;
}

ACTION tunnel_decap_copy_inner.copy_inner_ipv4_udp:
struct d {
}
phvwr phv[ipv4_version...ipv4_dstAddr], k.[inner_ipv4_version...inner_ipv4_dstAddr]
phvwr phv[udp_srcPort...udp_checksum], k.[inner_udp_srcPort...inner_udp_checksum]
phvwr phv.ipv4_valid, TRUE
phvwr phv.udp_valid, TRUE
phvwr.e phv.inner_ipv4_valid, FALSE
phvwr phv.inner_udp_valid, FALSE

ACTION tunnel_decap_copy_inner.copy_inner_ipv4_other:
struct d {
}
phvwr phv[ipv4_version...ipv4_dstAddr], k.[inner_ipv4_version...inner_ipv4_dstAddr]
phvwr phv.ipv4_valid, TRUE
phvwr.e phv.inner_ipv4_valid, FALSE
phvwr phv.udp_valid, FALSE

ACTION tunnel_decap_copy_inner.copy_inner_eth_ipv4_udp:
phvwr phv[ethernet_dstAddr...ethernet_etherType], k[inner_ethernet_dstAddr...inner_ethernet_etherType]
phvwr phv[ipv4_version...ipv4_dstAddr], k.[inner_ipv4_version...inner_ipv4_dstAddr]
phvwr phv[udp_srcPort...udp_checksum], k.[inner_udp_srcPort...inner_udp_checksum]
phvwr phv.ethernet_valid, TRUE
phvwr phv.vlan_tag, FALSE
phvwr phv.ipv4_valid, TRUE
phvwr phv.udp_valid, TRUE
phvwr phv.inner_ethernet_valid, FALSE
phvwr.e phv.inner_ipv4_valid, FALSE
phvwr phv.inner_udp_valid, FALSE

ACTION tunnel_decap_copy_inner.copy_inner_eth_ipv4_other:
phvwr phv[ethernet_dstAddr...ethernet_etherType], k[inner_ethernet_dstAddr...inner_ethernet_etherType]
phvwr phv[ipv4_version...ipv4_dstAddr], k.[inner_ipv4_version...inner_ipv4_dstAddr]
phvwr phv.ethernet_valid, TRUE
phvwr phv.vlan_tag, FALSE
phvwr phv.ipv4_valid, TRUE
phvwr phv.inner_ethernet_valid, FALSE
phvwr.e phv.inner_ipv4_valid, FALSE
phvwr phv.udp_valid, FALSE

Format 2 (IPv6 and non-IP):
struct k {
  phv.inner_ethernet_valid : 1;
  phv.inner_ipv6_valid : 1;
  phv.inner_udp_valid : 1;

  // 112 bits
  inner_ethernet_dstAddr : 48;
  inner_ethernet_srcAddr : 48;
  inner_ethernet_etherType : 16;

  // 320 bits
  inner_ipv6_version : 4;
  inner_ipv6_trafficClass : 8;
  inner_ipv6_flowLabel : 20;
  inner_ipv6_payloadLen : 16;
  inner_ipv6_nextHdr : 8;
  inner_ipv6_hopLimit : 8;
  inner_ipv6_srcAddr : 128;
  inner_ipv6_dstAddr : 128;

  // 64 bits
  inner_udp_srcPort : 16;
  inner_udp_dstPort : 16;
  inner_udp_len : 16;
  inner_udp_checksum : 16;
}

ACTION tunnel_decap_copy_inner.copy_inner_ipv6_udp:
phvwr phv[ipv6_version...ipv6_dstAddr], k.[inner_ipv6_version...inner_ipv6_dstAddr]
phvwr phv[udp_srcPort...udp_checksum], k.[inner_udp_srcPort...inner_udp_checksum]
phvwr phv.ipv6_valid, TRUE
phvwr phv.udp_valid, TRUE
phvwr.e phv.inner_ipv6_valid, FALSE
phvwr phv.inner_udp_valid, FALSE

ACTION tunnel_decap_copy_inner.copy_inner_ipv6_other:
phvwr phv[ipv6_version...ipv6_dstAddr], k.[inner_ipv6_version...inner_ipv6_dstAddr]
phvwr phv.ipv6_valid, TRUE
phvwr.e phv.inner_ipv6_valid, FALSE
phvwr phv.udp_valid, FALSE

ACTION tunnel_decap_copy_inner.copy_inner_eth_ipv6_udp:
phvwr phv[ethernet_dstAddr...ethernet_etherType], k[inner_ethernet_dstAddr...inner_ethernet_etherType]
phvwr phv[ipv6_version...ipv6_dstAddr], k.[inner_ipv6_version...inner_ipv6_dstAddr]
phvwr phv[udp_srcPort...udp_checksum], k.[inner_udp_srcPort...inner_udp_checksum]
phvwr phv.ethernet_valid, TRUE
phvwr phv.vlan_tag, FALSE
phvwr phv.ipv6_valid, TRUE
phvwr phv.udp_valid, TRUE
phvwr phv.inner_ethernet_valid, FALSE
phvwr.e phv.inner_ipv6_valid, FALSE
phvwr phv.inner_udp_valid, FALSE

ACTION tunnel_decap_copy_inner.copy_inner_eth_ipv6_other:
phvwr phv[ethernet_dstAddr...ethernet_etherType], k[inner_ethernet_dstAddr...inner_ethernet_etherType]
phvwr phv[ipv6_version...ipv6_dstAddr], k.[inner_ipv6_version...inner_ipv6_dstAddr]
phvwr phv.ethernet_valid, TRUE
phvwr phv.vlan_tag, FALSE
phvwr phv.ipv6_valid, TRUE
phvwr phv.inner_ethernet_valid, FALSE
phvwr.e phv.inner_ipv6_valid, FALSE
phvwr phv.udp_valid, FALSE

ACTION tunnel_decap_copy_inner.copy_inner_eth_non_ip:
phvwr phv[ethernet_dstAddr...ethernet_etherType], k[inner_ethernet_dstAddr...inner_ethernet_etherType]
phvwr phv.ethernet_valid, TRUE
phvwr phv.vlan_tag, FALSE
phvwr.e phv.inner_ethernet_valid, FALSE
phvwr phv.udp_valid, FALSE

/******************************************************************************/
TABLE : tunnel_decap
/******************************************************************************/
struct k {
}

ACTION tunnel_decap.remove_tunnel_hdrs:
struct d {
}
phvwr phv.vxlan_valid, FALSE
phvwr phv.genv_valid, FALSE
phvwr phv.nvgre_valid, FALSE
phvwr phv.gre_valid, FALSE
phvwr phv.mpls_0_valid, FALSE
phvwr.e phv.mpls_1_valid, FALSE
phvwr phv.mpls_2_valid, FALSE

/******************************************************************************/
TABLE : decap_vlan
/******************************************************************************/
struct k {
}
ACTION decap_vlan.decap_vlan:
struct d {
}
phvwr.e phv.ethernet_etherType, k.vlan_tag_etherType
phvwr phv.vlan_tag_valid, FALSE

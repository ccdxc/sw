/******************************************************************************/
TABLE : tunnel_rewrite
/******************************************************************************/
struct k {
  phv.tunnel_rewrite_index : 12;
  phv.l3_payload_length : 16;
  phv.entropy_hash : 16;
  phv.ethetnet_ethertype : 16;
  phv.mirror_session_id : 8;
  phv.header_valid_bits_hi : 64;
  phv.header_valid_bits_lo : 64;
}

ACTION tunnel_rewrite.encap_ipv4_vxlan:
struct d {
  // 112 bits
  ethernet_dstAddr : 48;
  ethernet_srcAddr : 48;
  ethernet_etherType : 16;

  // 160 bits
  ipv4_version : 4;
  ipv4_ihl : 4;
  ipv4_diffserv : 8;
  ipv4_totalLen : 16;
  ipv4_identification : 16;
  ipv4_flags : 3;
  ipv4_fragOffset : 13;
  ipv4_ttl : 8;
  ipv4_protocol : 8;
  ipv4_hdrChecksum : 16;
  ipv4_srcAddr : 32;
  ipv4_dstAddr: 32;

  // 64 bits
  udp_srcPort : 16;
  udp_dstPort : 16;
  udp_len : 16;
  udp_checksum : 16;

  // 64 bits
  vxlan_flags : 8;
  vxlan_reserved : 24;
  vxlan_vni : 24;
  vxlan_reserved2 : 8;
}
phvwr phv[ethernet_dstAddr...ethernet_etherType], d[ethernet_dstAddr...ethernet_etherType]
phvwr phv[ipv4_version...ipv4_dstAddr], d[ipv4_version...ipv4_dstAddr]
phvwr phv[udp_srcPort...udp_checksum], d[udp_srcPort...udp_checksum]
phvwr phv[vxlan_flags...vxlan_reserved2], d[vxlan_flags...vxlan_reserved2]
add r1, k.l3_payload_length, 50
phvwr phv.ipv4_totallen, r1
add r1, k.l3_payload_length, 30
phvwr phv.udp_len, r1
phvwr phv.udp_sport, k.entropy_hash
phvwr phv.ethernet_valid, TRUE
phvwr phv.ipv4_valid, TRUE
phvwr.e phv.udp_valid, TRUE
phvwr phv.vxlan_valid, TRUE

ACTION tunnel_rewrite.encap_ipv4_genv:
struct d {
  // 112 bits
  ethernet_dstAddr : 48;
  ethernet_srcAddr : 48;
  ethernet_etherType : 16;

  // 160 bits
  ipv4_version : 4;
  ipv4_ihl : 4;
  ipv4_diffserv : 8;
  ipv4_totalLen : 16;
  ipv4_identification : 16;
  ipv4_flags : 3;
  ipv4_fragOffset : 13;
  ipv4_ttl : 8;
  ipv4_protocol : 8;
  ipv4_hdrChecksum : 16;
  ipv4_srcAddr : 32;
  ipv4_dstAddr: 32;

  // 64 bits
  udp_srcPort : 16;
  udp_dstPort : 16;
  udp_len : 16;
  udp_checksum : 16;

  // 64 bits
  genv_ver : 2;
  genv_optLen : 6;
  genv_oam : 1;
  genv_critical : 1;
  genv_reserved : 6;
  genv_protoType : 16;
  genv_vni : 24;
  genv_reserved2 : 8;
}
phvwr phv[ethernet_dstAddr...ethernet_etherType], d[ethernet_dstAddr...ethernet_etherType]
phvwr phv[ipv4_version...ipv4_dstAddr], d[ipv4_version...ipv4_dstAddr]
phvwr phv[udp_srcPort...udp_checksum], d[udp_srcPort...udp_checksum]
phvwr phv[genv_ver...genv_reserved2], d[genv_ver...genv_reserved2]
add r1, k.l3_payload_length, 50
phvwr phv.ipv4_totallen, r1
add r1, k.l3_payload_length, 30
phvwr phv.udp_len, r1
phvwr phv.udp_sport, k.entropy_hash
phvwr phv.ethernet_valid, TRUE
phvwr phv.ipv4_valid, TRUE
phvwr.e phv.udp_valid, TRUE
phvwr phv.genv_valid, TRUE

ACTION tunnel_rewrite.encap_ipv4_nvgre:
struct d {
  // 112 bits
  ethernet_dstAddr : 48;
  ethernet_srcAddr : 48;
  ethernet_etherType : 16;

  // 160 bits
  ipv4_version : 4;
  ipv4_ihl : 4;
  ipv4_diffserv : 8;
  ipv4_totalLen : 16;
  ipv4_identification : 16;
  ipv4_flags : 3;
  ipv4_fragOffset : 13;
  ipv4_ttl : 8;
  ipv4_protocol : 8;
  ipv4_hdrChecksum : 16;
  ipv4_srcAddr : 32;
  ipv4_dstAddr: 32;

  // 32 bits
  gre_C : 1;
  gre_R : 1;
  gre_K : 1;
  gre_S : 1;
  gre_s : 1;
  gre_recurse : 3;
  gre_flags : 5;
  gre_ver : 3;
  gre_proto : 16;

  // 32 bits
  nvgre_tni : 24;
  nvgre_flow_id : 8;
}
phvwr phv[ethernet_dstAddr...ethernet_etherType], d[ethernet_dstAddr...ethernet_etherType]
phvwr phv[ipv4_version...ipv4_dstAddr], d[ipv4_version...ipv4_dstAddr]
phvwr phv[gre_C..gre_proto], d[gre_C...gre_proto]
phvwr phv[nvgre_tni...nvgre_flow_id], d[nvgre_tni...nvgre_flow_id]
add r1, k.l3_payload_length, 42
phvwr phv.ipv4_totallen, r1
phvwr phv.nvgre_flow_id, k.entropy_hash
phvwr phv.ethernet_valid, TRUE
phvwr phv.ipv4_valid, TRUE
phvwr.e phv.gre_valid, TRUE
phvwr phv.nvgre_valid, TRUE

ACTION tunnel_rewrite.encap_ipv4_gre:
struct d {
  // 112 bits
  ethernet_dstAddr : 48;
  ethernet_srcAddr : 48;
  ethernet_etherType : 16;

  // 160 bits
  ipv4_version : 4;
  ipv4_ihl : 4;
  ipv4_diffserv : 8;
  ipv4_totalLen : 16;
  ipv4_identification : 16;
  ipv4_flags : 3;
  ipv4_fragOffset : 13;
  ipv4_ttl : 8;
  ipv4_protocol : 8;
  ipv4_hdrChecksum : 16;
  ipv4_srcAddr : 32;
  ipv4_dstAddr: 32;

  // 32 bits
  gre_ C : 1;
  gre_ R : 1;
  gre_ K : 1;
  gre_ S : 1;
  gre_ s : 1;
  gre_ recurse : 3;
  gre_ flags : 5;
  gre_ ver : 3;
  gre_ proto : 16;
}
phvwr phv[ethernet_dstAddr...ethernet_etherType], d[ethernet_dstAddr...ethernet_etherType]
phvwr phv[ipv4_version...ipv4_dstAddr], d[ipv4_version...ipv4_dstAddr]
phvwr phv[gre_C..gre_proto], d[gre_C...gre_proto]
add r1, k.l3_payload_length, 24
phvwr phv.ipv4_totallen, r1
phvwr phv.gre_proto, k.ethernet_ethertype
phvwr phv.ethernet_valid, TRUE
phvwr.e phv.ipv4_valid, TRUE
phvwr phv.gre_valid, TRUE

ACTION tunnel_rewrite.encap_ipv4_ip:
struct d {
  // 112 bits
  ethernet_dstAddr : 48;
  ethernet_srcAddr : 48;
  ethernet_etherType : 16;

  // 160 bits
  ipv4_version : 4;
  ipv4_ihl : 4;
  ipv4_diffserv : 8;
  ipv4_totalLen : 16;
  ipv4_identification : 16;
  ipv4_flags : 3;
  ipv4_fragOffset : 13;
  ipv4_ttl : 8;
  ipv4_protocol : 8;
  ipv4_hdrChecksum : 16;
  ipv4_srcAddr : 32;
  ipv4_dstAddr: 32;
}
phvwr phv[ethernet_dstAddr...ethernet_etherType], d[ethernet_dstAddr...ethernet_etherType]
phvwr phv[ipv4_version...ipv4_dstAddr], d[ipv4_version...ipv4_dstAddr]
add r1, k.l3_payload_length, 20
phvwr.e phv.ipv4_totallen, r1
phvwr phv.ipv4_valid, TRUE

ACTION tunnel_rewrite.encap_ipv6_vxlan:
struct d {
  // 112 bits
  ethernet_dstAddr : 48;
  ethernet_srcAddr : 48;
  ethernet_etherType : 16;

  // 320 bits
  ipv6_version : 4;
  ipv6_trafficClass : 8;
  ipv6_flowLabel : 20;
  ipv6_payloadLen : 16;
  ipv6_nextHdr : 8;
  ipv6_hopLimit : 8;
  ipv6_srcAddr : 128;
  ipv6_dstAddr : 128;

  // 32 bits
  udp_srcPort : 16;
  udp_dstPort : 16;

  // 32 bits
  vxlan_vni : 24;
  vxlan_reserved2 : 8;
}
phvwr phv[ethernet_dstAddr...ethernet_etherType], d[ethernet_dstAddr...ethernet_etherType]
phvwr phv[ipv6_version...ipv6_dstAddr], d[ipv6_version...ipv6_dstAddr]
phvwr phv[udp_srcPort...udp_dstPort], d[udp_srcPort...udp_dstPort]
phvwr phv[vxlan_vni...vxlan_reserved2], d[vxlan_vni...vxlan_reserved2]
phvwr phv.udp_checksum, 0
phvwr phv.[vxlan_flags...vxlan_reserved2], 0x8000
add r1, k.l3_payload_length, 30
phvwr phv.ipv6_payloadlen, r1
phvwr phv.udp_len, r1
phvwr phv.udp_sport, k.entropy_hash
phvwr phv.ethernet_valid, TRUE
phvwr phv.ipv6_valid, TRUE
phvwr.e phv.udp_valid, TRUE
phvwr phv.vxlan_valid, TRUE

ACTION tunnel_rewrite.encap_ipv6_genv:
struct d {
  // 96 bits
  ethernet_dstAddr : 48;
  ethernet_srcAddr : 48;

  // 320 bits
  ipv6_version : 4;
  ipv6_trafficClass : 8;
  ipv6_flowLabel : 20;
  ipv6_payloadLen : 16;
  ipv6_nextHdr : 8;
  ipv6_hopLimit : 8;
  ipv6_srcAddr : 128;
  ipv6_dstAddr : 128;

  // 32 bits
  udp_srcPort : 16;
  udp_dstPort : 16;

  // 64 bits
  genv_ver : 2;
  genv_optLen : 6;
  genv_oam : 1;
  genv_critical : 1;
  genv_reserved : 6;
  genv_protoType : 16;
  genv_vni : 24;
  genv_reserved2 : 8;
}
phvwr phv[ethernet_dstAddr...ethernet_srcAddr], d[ethernet_dstAddr...ethernet_srcAddr]
phvwr phv[ipv6_version...ipv6_dstAddr], d[ipv6_version...ipv6_dstAddr]
phvwr phv[udp_srcPort...udp_dstPort], d[udp_srcPort...udp_dstPort]
phvwr phv[genv_ver...genv_reserved2], d[genv_ver...genv_reserved2]
phvwr phv.udp_checksum, 0
phvwr phv.ethernet_ethertype, ETHERNET_IPV6
add r1, k.l3_payload_length, 30
phvwr phv.ipv6_payloadlen, r1
phvwr phv.udp_len, r1
phvwr phv.udp_sport, k.entropy_hash
phvwr phv.ethernet_valid, TRUE
phvwr phv.ipv6_valid, TRUE
phvwr.e phv.udp_valid, TRUE
phvwr phv.genv_valid, TRUE

ACTION tunnel_rewrite.encap_ipv6_nvgre:
struct d {
  // 112 bits
  ethernet_dstAddr : 48;
  ethernet_srcAddr : 48;
  ethernet_etherType : 16;

  // 320 bits
  ipv6_version : 4;
  ipv6_trafficClass : 8;
  ipv6_flowLabel : 20;
  ipv6_payloadLen : 16;
  ipv6_nextHdr : 8;
  ipv6_hopLimit : 8;
  ipv6_srcAddr : 128;
  ipv6_dstAddr : 128;

  // 32 bits
  gre_C : 1;
  gre_R : 1;
  gre_K : 1;
  gre_S : 1;
  gre_s : 1;
  gre_recurse : 3;
  gre_flags : 5;
  gre_ver : 3;
  gre_proto : 16;

  // 32 bits
  nvgre_tni : 24;
  nvgre_flow_id : 8;
}
phvwr phv[ethernet_dstAddr...ethernet_etherType], d[ethernet_dstAddr...ethernet_etherType]
phvwr phv[ipv6_version...ipv6_dstAddr], d[ipv6_version...ipv6_dstAddr]
phvwr phv[gre_C..gre_proto], d[gre_C...gre_proto]
phvwr phv[nvgre_tni...nvgre_flow_id], d[nvgre_tni...nvgre_flow_id]
add r1, k.l3_payload_length, 22
phvwr phv.ipv6_payloadlen, r1
phvwr phv.nvgre_flow_id, k.entropy_hash
phvwr phv.ethernet_valid, TRUE
phvwr phv.ipv6_valid, TRUE
phvwr.e phv.gre_valid, TRUE
phvwr phv.nvgre_valid, TRUE

ACTION tunnel_rewrite.encap_ipv6_gre:
struct d {
  // 112 bits
  ethernet_dstAddr : 48;
  ethernet_srcAddr : 48;
  ethernet_etherType : 16;

  // 320 bits
  ipv6_version : 4;
  ipv6_trafficClass : 8;
  ipv6_flowLabel : 20;
  ipv6_payloadLen : 16;
  ipv6_nextHdr : 8;
  ipv6_hopLimit : 8;
  ipv6_srcAddr : 128;
  ipv6_dstAddr : 128;

  // 32 bits
  gre_C : 1;
  gre_R : 1;
  gre_K : 1;
  gre_S : 1;
  gre_s : 1;
  gre_recurse : 3;
  gre_flags : 5;
  gre_ver : 3;
  gre_proto : 16;
}
phvwr phv[ethernet_dstAddr...ethernet_etherType], d[ethernet_dstAddr...ethernet_etherType]
phvwr phv[ipv6_version...ipv6_dstAddr], d[ipv6_version...ipv6_dstAddr]
phvwr phv[gre_C..gre_proto], d[gre_C...gre_proto]
add r1, k.l3_payload_length, 4
phvwr phv.ipv6_payloadlen, r1
phvwr phv.gre_proto, k.ethernet_ethertype
phvwr phv.ethernet_valid, TRUE
phvwr.e phv.ipv6_valid, TRUE
phvwr phv.gre_valid, TRUE

ACTION tunnel_rewrite.encap_ipv6_ip:
struct d {
  // 112 bits
  ethernet_dstAddr : 48;
  ethernet_srcAddr : 48;
  ethernet_etherType : 16;

  // 320 bits
  ipv6_version : 4;
  ipv6_trafficClass : 8;
  ipv6_flowLabel : 20;
  ipv6_payloadLen : 16;
  ipv6_nextHdr : 8;
  ipv6_hopLimit : 8;
  ipv6_srcAddr : 128;
  ipv6_dstAddr : 128;
}
phvwr phv[ethernet_dstAddr...ethernet_etherType], d[ethernet_dstAddr...ethernet_etherType]
phvwr phv[ipv6_version...ipv6_dstAddr], d[ipv6_version...ipv6_dstAddr]
phvwr.e phv.ipv6_payloadlen, k.l3_payload_length
phvwr phv.ipv6_valid, TRUE

ACTION tunnel_rewrite.encap_vlan_ipv4_vxlan:
struct d {
  // 112 bits
  ethernet_dstAddr : 48;
  ethernet_srcAddr : 48;
  ethernet_etherType : 16;

  // 32 bits
  vlan_pcp : 3;
  vlan_cfi : 1;
  vlan_vid : 12;
  vlan_etherType : 16;

  // 160 bits
  ipv4_version : 4;
  ipv4_ihl : 4;
  ipv4_diffserv : 8;
  ipv4_totalLen : 16;
  ipv4_identification : 16;
  ipv4_flags : 3;
  ipv4_fragOffset : 13;
  ipv4_ttl : 8;
  ipv4_protocol : 8;
  ipv4_hdrChecksum : 16;
  ipv4_srcAddr : 32;
  ipv4_dstAddr: 32;

  // 64 bits
  udp_srcPort : 16;
  udp_dstPort : 16;
  udp_len : 16;
  udp_checksum : 16;

  // 64 bits
  vxlan_flags : 8;
  vxlan_reserved : 24;
  vxlan_vni : 24;
  vxlan_reserved2 : 8;
}
phvwr phv[ethernet_dstAddr...ethernet_etherType], d[ethernet_dstAddr...ethernet_etherType]
phvwr phv[vlan_pcp...vlan_etherType], d[vlan_pcp...vlan_etherType]
phvwr phv[ipv4_version...ipv4_dstAddr], d[ipv4_version...ipv4_dstAddr]
phvwr phv[udp_srcPort...udp_checksum], d[udp_srcPort...udp_checksum]
phvwr phv[vxlan_flags...vxlan_reserved2], d[vxlan_flags...vxlan_reserved2]
add r1, k.l3_payload_length, 50
phvwr phv.ipv4_totallen, r1
add r1, k.l3_payload_length, 30
phvwr phv.udp_len, r1
phvwr phv.udp_sport, k.entropy_hash
phvwr phv.ethernet_valid, TRUE
phvwr phv.vlan_valid, TRUE
phvwr phv.ipv4_valid, TRUE
phvwr.e phv.udp_valid, TRUE
phvwr phv.vxlan_valid, TRUE

ACTION tunnel_rewrite.encap_vlan_ipv4_genv:
struct d {
  // 112 bits
  ethernet_dstAddr : 48;
  ethernet_srcAddr : 48;
  ethernet_etherType : 16;

  // 32 bits
  vlan_pcp : 3;
  vlan_cfi : 1;
  vlan_vid : 12;
  vlan_etherType : 16;

  // 160 bits
  ipv4_version : 4;
  ipv4_ihl : 4;
  ipv4_diffserv : 8;
  ipv4_totalLen : 16;
  ipv4_identification : 16;
  ipv4_flags : 3;
  ipv4_fragOffset : 13;
  ipv4_ttl : 8;
  ipv4_protocol : 8;
  ipv4_hdrChecksum : 16;
  ipv4_srcAddr : 32;
  ipv4_dstAddr: 32;

  // 64 bits
  udp_srcPort : 16;
  udp_dstPort : 16;
  udp_len : 16;
  udp_checksum : 16;

  // 64 bits
  genv_ver : 2;
  genv_optLen : 6;
  genv_oam : 1;
  genv_critical : 1;
  genv_reserved : 6;
  genv_protoType : 16;
  genv_vni : 24;
  genv_reserved2 : 8;
}
phvwr phv[ethernet_dstAddr...ethernet_etherType], d[ethernet_dstAddr...ethernet_etherType]
phvwr phv[vlan_pcp...vlan_etherType], d[vlan_pcp...vlan_etherType]
phvwr phv[ipv4_version...ipv4_dstAddr], d[ipv4_version...ipv4_dstAddr]
phvwr phv[udp_srcPort...udp_checksum], d[udp_srcPort...udp_checksum]
phvwr phv[genv_ver...genv_reserved2], d[genv_ver...genv_reserved2]
add r1, k.l3_payload_length, 50
phvwr phv.ipv4_totallen, r1
add r1, k.l3_payload_length, 30
phvwr phv.udp_len, r1
phvwr phv.udp_sport, k.entropy_hash
phvwr phv.ethernet_valid, TRUE
phvwr phv.vlan_valid, TRUE
phvwr phv.ipv4_valid, TRUE
phvwr.e phv.udp_valid, TRUE
phvwr phv.genv_valid, TRUE

ACTION tunnel_rewrite.encap_vlan_ipv4_nvgre:
struct d {
  // 112 bits
  ethernet_dstAddr : 48;
  ethernet_srcAddr : 48;
  ethernet_etherType : 16;

  // 32 bits
  vlan_pcp : 3;
  vlan_cfi : 1;
  vlan_vid : 12;
  vlan_etherType : 16;

  // 160 bits
  ipv4_version : 4;
  ipv4_ihl : 4;
  ipv4_diffserv : 8;
  ipv4_totalLen : 16;
  ipv4_identification : 16;
  ipv4_flags : 3;
  ipv4_fragOffset : 13;
  ipv4_ttl : 8;
  ipv4_protocol : 8;
  ipv4_hdrChecksum : 16;
  ipv4_srcAddr : 32;
  ipv4_dstAddr: 32;

  // 32 bits
  gre_C : 1;
  gre_R : 1;
  gre_K : 1;
  gre_S : 1;
  gre_s : 1;
  gre_recurse : 3;
  gre_flags : 5;
  gre_ver : 3;
  gre_proto : 16;

  // 32 bits
  nvgre_tni : 24;
  nvgre_flow_id : 8;
}
phvwr phv[ethernet_dstAddr...ethernet_etherType], d[ethernet_dstAddr...ethernet_etherType]
phvwr phv[vlan_pcp...vlan_etherType], d[vlan_pcp...vlan_etherType]
phvwr phv[ipv4_version...ipv4_dstAddr], d[ipv4_version...ipv4_dstAddr]
phvwr phv[gre_C..gre_proto], d[gre_C...gre_proto]
phvwr phv[nvgre_tni...nvgre_flow_id], d[nvgre_tni...nvgre_flow_id]
add r1, k.l3_payload_length, 42
phvwr phv.ipv4_totallen, r1
phvwr phv.nvgre_flow_id, k.entropy_hash
phvwr phv.ethernet_valid, TRUE
phvwr phv.vlan_valid, TRUE
phvwr phv.ipv4_valid, TRUE
phvwr.e phv.gre_valid, TRUE
phvwr phv.nvgre_valid, TRUE

ACTION tunnel_rewrite.encap_vlan_ipv4_gre:
struct d {
  // 112 bits
  ethernet_dstAddr : 48;
  ethernet_srcAddr : 48;
  ethernet_etherType : 16;

  // 32 bits
  vlan_pcp : 3;
  vlan_cfi : 1;
  vlan_vid : 12;
  vlan_etherType : 16;

  // 160 bits
  ipv4_version : 4;
  ipv4_ihl : 4;
  ipv4_diffserv : 8;
  ipv4_totalLen : 16;
  ipv4_identification : 16;
  ipv4_flags : 3;
  ipv4_fragOffset : 13;
  ipv4_ttl : 8;
  ipv4_protocol : 8;
  ipv4_hdrChecksum : 16;
  ipv4_srcAddr : 32;
  ipv4_dstAddr: 32;

  // 32 bits
  gre_ C : 1;
  gre_ R : 1;
  gre_ K : 1;
  gre_ S : 1;
  gre_ s : 1;
  gre_ recurse : 3;
  gre_ flags : 5;
  gre_ ver : 3;
  gre_ proto : 16;
}
phvwr phv[ethernet_dstAddr...ethernet_etherType], d[ethernet_dstAddr...ethernet_etherType]
phvwr phv[vlan_pcp...vlan_etherType], d[vlan_pcp...vlan_etherType]
phvwr phv[ipv4_version...ipv4_dstAddr], d[ipv4_version...ipv4_dstAddr]
phvwr phv[gre_C..gre_proto], d[gre_C...gre_proto]
add r1, k.l3_payload_length, 24
phvwr phv.ipv4_totallen, r1
phvwr phv.gre_proto, k.ethernet_ethertype
phvwr phv.ethernet_valid, TRUE
phvwr phv.vlan_valid, TRUE
phvwr.e phv.ipv4_valid, TRUE
phvwr phv.gre_valid, TRUE

ACTION tunnel_rewrite.encap_vlan_ipv4_ip:
struct d {
  // 112 bits
  ethernet_dstAddr : 48;
  ethernet_srcAddr : 48;
  ethernet_etherType : 16;

  // 32 bits
  vlan_pcp : 3;
  vlan_cfi : 1;
  vlan_vid : 12;
  vlan_etherType : 16;

  // 160 bits
  ipv4_version : 4;
  ipv4_ihl : 4;
  ipv4_diffserv : 8;
  ipv4_totalLen : 16;
  ipv4_identification : 16;
  ipv4_flags : 3;
  ipv4_fragOffset : 13;
  ipv4_ttl : 8;
  ipv4_protocol : 8;
  ipv4_hdrChecksum : 16;
  ipv4_srcAddr : 32;
  ipv4_dstAddr: 32;
}
phvwr phv[ethernet_dstAddr...ethernet_etherType], d[ethernet_dstAddr...ethernet_etherType]
phvwr phv[vlan_pcp...vlan_etherType], d[vlan_pcp...vlan_etherType]
phvwr phv[ipv4_version...ipv4_dstAddr], d[ipv4_version...ipv4_dstAddr]
add r1, k.l3_payload_length, 20
phvwr.e phv.ipv4_totallen, r1
phvwr phv.vlan_valid, TRUE
phvwr phv.ipv4_valid, TRUE

ACTION tunnel_rewrite.encap_vlan_ipv6_vxlan:
struct d {
  // 112 bits
  ethernet_dstAddr : 48;
  ethernet_srcAddr : 48;
  ethernet_etherType : 16;

  // 16 bits
  vlan_pcp : 3;
  vlan_cfi : 1;
  vlan_vid : 12;

  // 320 bits
  ipv6_version : 4;
  ipv6_trafficClass : 8;
  ipv6_flowLabel : 20;
  ipv6_payloadLen : 16;
  ipv6_nextHdr : 8;
  ipv6_hopLimit : 8;
  ipv6_srcAddr : 128;
  ipv6_dstAddr : 128;

  // 32 bits
  udp_srcPort : 16;
  udp_dstPort : 16;

  // 32 bits
  vxlan_vni : 24;
  vxlan_reserved2 : 8;
}
phvwr phv[ethernet_dstAddr...ethernet_etherType], d[ethernet_dstAddr...ethernet_etherType]
phvwr phv[vlan_pcp...vlan_vid], d[vlan_pcp...vlan_vid]
phvwr phv[ipv6_version...ipv6_dstAddr], d[ipv6_version...ipv6_dstAddr]
phvwr phv[udp_srcPort...udp_dstPort], d[udp_srcPort...udp_dstPort]
phvwr phv[vxlan_vni...vxlan_reserved2], d[vxlan_vni...vxlan_reserved2]
phvwr phv.udp_checksum, 0
phvwr phv.[vxlan_flags...vxlan_reserved2], 0x8000
phvwr phv.vlan_etherType, ETHERTYPE_IPV6
add r1, k.l3_payload_length, 30
phvwr phv.ipv6_payloadlen, r1
phvwr phv.udp_len, r1
phvwr phv.udp_sport, k.entropy_hash
phvwr phv.ethernet_valid, TRUE
phvwr phv.vlan_valid, TRUE
phvwr phv.ipv6_valid, TRUE
phvwr.e phv.udp_valid, TRUE
phvwr phv.vxlan_valid, TRUE

ACTION tunnel_rewrite.encap_vlan_ipv6_genv:
struct d {
  // 96 bits
  ethernet_dstAddr : 48;
  ethernet_srcAddr : 48;

  // 16 bits
  vlan_pcp : 3;
  vlan_cfi : 1;
  vlan_vid : 12;

  // 304 bits
  ipv6_version : 4;
  ipv6_trafficClass : 8;
  ipv6_flowLabel : 20;

  ipv6_nextHdr : 8;
  ipv6_hopLimit : 8;
  ipv6_srcAddr : 128;
  ipv6_dstAddr : 128;

  // 32 bits
  udp_srcPort : 16;
  udp_dstPort : 16;

  // 64 bits
  genv_ver : 2;
  genv_optLen : 6;
  genv_oam : 1;
  genv_critical : 1;
  genv_reserved : 6;
  genv_protoType : 16;
  genv_vni : 24;
  genv_reserved2 : 8;
}
phvwr phv[ethernet_dstAddr...ethernet_srcAddr], d[ethernet_dstAddr...ethernet_srcAddr]
phvwr phv[vlan_pcp...vlan_vid], d[vlan_pcp...vlan_vid]
phvwr phv[ipv6_version...ipv6_flowLabel], d[ipv6_version...ipv6_flowLabel]
phvwr phv[ipv6_nextHdr...ipv6_dstAddr], d[ipv6_nextHdr...ipv6_dstAddr]
phvwr phv[udp_srcPort...udp_dstPort], d[udp_srcPort...udp_dstPort]
phvwr phv[genv_ver...genv_reserved2], d[genv_ver...genv_reserved2]
phvwr phv.udp_checksum, 0
phvwr phv.ethernet_ethertype, ETHERNET_VLAN
phvwr phv.vlan_etherType, ETHERTYPE_IPV6
add r1, k.l3_payload_length, 30
phvwr phv.ipv6_payloadlen, r1
phvwr phv.udp_len, r1
phvwr phv.udp_sport, k.entropy_hash
phvwr phv.ethernet_valid, TRUE
phvwr phv.vlan_valid, TRUE
phvwr phv.ipv6_valid, TRUE
phvwr.e phv.udp_valid, TRUE
phvwr phv.genv_valid, TRUE

ACTION tunnel_rewrite.encap_vlan_ipv6_nvgre:
struct d {
  // 112 bits
  ethernet_dstAddr : 48;
  ethernet_srcAddr : 48;
  ethernet_etherType : 16;

  // 16 bits
  vlan_pcp : 3;
  vlan_cfi : 1;
  vlan_vid : 12;

  // 320 bits
  ipv6_version : 4;
  ipv6_trafficClass : 8;
  ipv6_flowLabel : 20;
  ipv6_payloadLen : 16;
  ipv6_nextHdr : 8;
  ipv6_hopLimit : 8;
  ipv6_srcAddr : 128;
  ipv6_dstAddr : 128;

  // 32 bits
  gre_C : 1;
  gre_R : 1;
  gre_K : 1;
  gre_S : 1;
  gre_s : 1;
  gre_recurse : 3;
  gre_flags : 5;
  gre_ver : 3;
  gre_proto : 16;

  // 32 bits
  nvgre_tni : 24;
  nvgre_flow_id : 8;
}
phvwr phv[ethernet_dstAddr...ethernet_etherType], d[ethernet_dstAddr...ethernet_etherType]
phvwr phv[vlan_pcp...vlan_vid], d[vlan_pcp...vlan_vid]
phvwr phv[ipv6_version...ipv6_dstAddr], d[ipv6_version...ipv6_dstAddr]
phvwr phv[gre_C..gre_proto], d[gre_C...gre_proto]
phvwr phv[nvgre_tni...nvgre_flow_id], d[nvgre_tni...nvgre_flow_id]
add r1, k.l3_payload_length, 22
phvwr phv.ipv6_payloadlen, r1
phvwr phv.nvgre_flow_id, k.entropy_hash
phvwr phv.vlan_etherType, ETHERTYPE_IPV6
phvwr phv.ethernet_valid, TRUE
phvwr phv.vlan_valid, TRUE
phvwr phv.ipv6_valid, TRUE
phvwr.e phv.gre_valid, TRUE
phvwr phv.nvgre_valid, TRUE

ACTION tunnel_rewrite.encap_vlan_ipv6_gre:
struct d {
  // 112 bits
  ethernet_dstAddr : 48;
  ethernet_srcAddr : 48;
  ethernet_etherType : 16;

  // 32 bits
  vlan_pcp : 3;
  vlan_cfi : 1;
  vlan_vid : 12;
  vlan_etherType : 16;

  // 320 bits
  ipv6_version : 4;
  ipv6_trafficClass : 8;
  ipv6_flowLabel : 20;
  ipv6_payloadLen : 16;
  ipv6_nextHdr : 8;
  ipv6_hopLimit : 8;
  ipv6_srcAddr : 128;
  ipv6_dstAddr : 128;

  // 32 bits
  gre_C : 1;
  gre_R : 1;
  gre_K : 1;
  gre_S : 1;
  gre_s : 1;
  gre_recurse : 3;
  gre_flags : 5;
  gre_ver : 3;
  gre_proto : 16;
}
phvwr phv[ethernet_dstAddr...ethernet_etherType], d[ethernet_dstAddr...ethernet_etherType]
phvwr phv[vlan_pcp...vlan_etherType], d[vlan_pcp...vlan_etherType]
phvwr phv[ipv6_version...ipv6_dstAddr], d[ipv6_version...ipv6_dstAddr]
phvwr phv[gre_C..gre_proto], d[gre_C...gre_proto]
add r1, k.l3_payload_length, 4
phvwr phv.ipv6_payloadlen, r1
phvwr phv.gre_proto, k.ethernet_ethertype
phvwr phv.ethernet_valid, TRUE
phvwr phv.vlan_valid, TRUE
phvwr.e phv.ipv6_valid, TRUE
phvwr phv.gre_valid, TRUE

ACTION tunnel_rewrite.encap_vlan_ipv6_ip:
struct d {
  // 112 bits
  ethernet_dstAddr : 48;
  ethernet_srcAddr : 48;
  ethernet_etherType : 16;

  // 32 bits
  vlan_pcp : 3;
  vlan_cfi : 1;
  vlan_vid : 12;
  vlan_etherType : 16;

  // 320 bits
  ipv6_version : 4;
  ipv6_trafficClass : 8;
  ipv6_flowLabel : 20;
  ipv6_payloadLen : 16;
  ipv6_nextHdr : 8;
  ipv6_hopLimit : 8;
  ipv6_srcAddr : 128;
  ipv6_dstAddr : 128;
}
phvwr phv[ethernet_dstAddr...ethernet_etherType], d[ethernet_dstAddr...ethernet_etherType]
phvwr phv[vlan_pcp...vlan_etherType], d[vlan_pcp...vlan_etherType]
phvwr phv[ipv6_version...ipv6_dstAddr], d[ipv6_version...ipv6_dstAddr]
phvwr.e phv.ipv6_payloadlen, k.l3_payload_length
phvwr phv.vlan_valid, TRUE
phvwr phv.ipv6_valid, TRUE

ACTION tunnel_rewrite.encap_ipv4_erspan:
struct d {
  // 112 bits
  ethernet_dstAddr : 48;
  ethernet_srcAddr : 48;
  ethernet_etherType : 16;

  // 160 bits
  ipv4_version : 4;
  ipv4_ihl : 4;
  ipv4_diffserv : 8;
  ipv4_totalLen : 16;
  ipv4_identification : 16;
  ipv4_flags : 3;
  ipv4_fragOffset : 13;
  ipv4_ttl : 8;
  ipv4_protocol : 8;
  ipv4_hdrChecksum : 16;
  ipv4_srcAddr : 32;
  ipv4_dstAddr: 32;

  // 32 bits
  gre_ C : 1;
  gre_ R : 1;
  gre_ K : 1;
  gre_ S : 1;
  gre_ s : 1;
  gre_ recurse : 3;
  gre_ flags : 5;
  gre_ ver : 3;
  gre_ proto : 16;

  // 96 bits
  erspan_version : 4;
  erspan_vlan : 12;
  erspan_priority : 6;
  erspan_span_id : 10;
  erspan_timestamp : 32;
  erspan_sgt       : 16;
  erspan_ft_d_other: 16;
}
phvwr phv[ethernet_dstAddr...ethernet_etherType], d[ethernet_dstAddr...ethernet_etherType]
phvwr phv[ipv4_version...ipv4_dstAddr], d[ipv4_version...ipv4_dstAddr]
phvwr phv[gre_C..gre_proto], d[gre_C...gre_proto]
phvwr phv[erspan_version...erspan_ft_d_other], d[erspan_version...erspan_ft_d_other]
add r1, k.l3_payload_length, 50
phvwr phv.ipv4_totallen, r1
phvwr phv.erspan_span_id, k.mirror_session_id
phvwr phv.erspan_timestamp, rX // timestamp register
phvwr phv.ethernet_valid, TRUE
phvwr phv.ipv4_valid, TRUE
phvwr.e phv.gre_valid, TRUE
phvwr phv.erspan_valid, TRUE

ACTION tunnel_rewrite.encap_ipv6_erspan:
struct d {
  // 112 bits
  ethernet_dstAddr : 48;
  ethernet_srcAddr : 48;
  ethernet_etherType : 16;

  // 320 bits
  ipv6_version : 4;
  ipv6_trafficClass : 8;
  ipv6_flowLabel : 20;
  ipv6_payloadLen : 16;
  ipv6_nextHdr : 8;
  ipv6_hopLimit : 8;
  ipv6_srcAddr : 128;
  ipv6_dstAddr : 128;

  // 32 bits
  gre_C : 1;
  gre_R : 1;
  gre_K : 1;
  gre_S : 1;
  gre_s : 1;
  gre_recurse : 3;
  gre_flags : 5;
  gre_ver : 3;
  gre_proto : 16;

  // 22 bits
  erspan_version : 4;
  erspan_vlan : 12;
  erspan_priority : 6;
}
phvwr phv[ethernet_dstAddr...ethernet_etherType], d[ethernet_dstAddr...ethernet_etherType]
phvwr phv[ipv6_version...ipv6_dstAddr], d[ipv6_version...ipv6_dstAddr]
phvwr phv[gre_C..gre_proto], d[gre_C...gre_proto]
phvwr phv[erspan_version...erspan_priority], d[erspan_version...priority]
phvwr phv[erspan_sgt...erspan_ft_d_other], 0
add r1, k.l3_payload_length, 26
phvwr phv.ipv6_payloadlen, r1
phvwr phv.erspan_span_id, k.mirror_session_id
phvwr phv.erspan_timestamp, rX // timestamp register
phvwr phv.ethernet_valid, TRUE
phvwr phv.ipv6_valid, TRUE
phvwr.e phv.gre_valid, TRUE
phvwr phv.erspan_valid, TRUE

ACTION tunnel_rewrite.encap_ip_mpls1:
ACTION tunnel_rewrite.encap_eompls1:
struct d {
  // 112 bits
  ethernet_dstAddr : 48;
  ethernet_srcAddr : 48;
  ethernet_etherType : 16;

  // mpls label
  mpls_0_label : 20;
  mpls_0_exp : 3;
  mpls_0_bos : 1;
  mpls_0_ttl : 8;
}
phvwr phv[ethernet_dstAddr...ethernet_etherType], d[ethernet_dstAddr...ethernet_etherType]
phvwr phv[mpls_0_label...mpls_0_ttl], d[mpls_0_label...mpls_0_ttl]
phvwr.e phv.ethernet_valid, TRUE
phvwr phv.mpls_0_valid, TRUE

ACTION tunnel_rewrite.encap_ip_mpls2:
ACTION tunnel_rewrite.encap_eompls2:
struct d {
  // 112 bits
  ethernet_dstAddr : 48;
  ethernet_srcAddr : 48;
  ethernet_etherType : 16;

  // mpls label
  mpls_0_label : 20;
  mpls_0_exp : 3;
  mpls_0_bos : 1;
  mpls_0_ttl : 8;

  // mpls label
  mpls_1_label : 20;
  mpls_1_exp : 3;
  mpls_1_bos : 1;
  mpls_1_ttl : 8;
}
phvwr phv[ethernet_dstAddr...ethernet_etherType], d[ethernet_dstAddr...ethernet_etherType]
phvwr phv[mpls_0_label...mpls_1_ttl], d[mpls_0_label...mpls_1_ttl]
phvwr phv.ethernet_valid, TRUE
phvwr.e phv.mpls_0_valid, TRUE
phvwr phv.mpls_1_valid, TRUE

ACTION tunnel_rewrite.encap_ip_mpls3:
ACTION tunnel_rewrite.encap_eompls3:
struct d {
  // 112 bits
  ethernet_dstAddr : 48;
  ethernet_srcAddr : 48;
  ethernet_etherType : 16;

  // mpls label
  mpls_0_label : 20;
  mpls_0_exp : 3;
  mpls_0_bos : 1;
  mpls_0_ttl : 8;

  // mpls label
  mpls_1_label : 20;
  mpls_1_exp : 3;
  mpls_1_bos : 1;
  mpls_1_ttl : 8;

  // mpls label
  mpls_2_label : 20;
  mpls_2_exp : 3;
  mpls_2_bos : 1;
  mpls_2_ttl : 8;
}
phvwr phv[ethernet_dstAddr...ethernet_etherType], d[ethernet_dstAddr...ethernet_etherType]
phvwr phv[mpls_0_label...mpls_2_ttl], d[mpls_0_label...mpls_2_ttl]
phvwr phv.ethernet_valid, TRUE
phvwr phv.mpls_0_valid, TRUE
phvwr.e phv.mpls_1_valid, TRUE
phvwr phv.mpls_2_valid, TRUE

ACTION tunnelp_rewrite.encap_vlan:
struct d {
  // 32 bits
  vlan_pcp : 3;
  vlan_cfi : 1;
  vlan_vid : 12;
}
phvwr phv.ethernet_etherType, ETHERTYPE_VLAN
phvwr phv[vlan_pcp...vlan_vid], d[vlan_pcp...vlan_vid]
phvwr.e phv.vlan_etherType, k.ethernet.etherType
phvwr phv.vlan_valid, TRUE

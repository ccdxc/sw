/******************************************************************************/
TABLE : tunnel_encap_update_inner
/******************************************************************************/
Key format 1 (IPv4):
struct k {
  phv.tcp_valid : 1;
  phv.udp_valid : 1;
  phv.icmp_valid : 1;
  phv.nat_update_checksum : 1;

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
}

ACTION tunnel_encap_update_inner.encap_inner_ipv4_udp_rewrite:
struct d {
}
phvwr phv[inner_ipv4_version...inner_ipv4_dstAddr], k[ipv4_version...ipv4_dstAddr]
phvwr phv[inner_udp_srcPort...inner_udp_checksum], k[udp_srcPort...udp_checksum]
phvwr phv.l3_payload_length, k.ipv4_totallen;
phvwr phv.tunnel_inner_ip_proto, IP_PROTOCOLS_IPV4
phvwr phv.inner_ipv4_valid, TRUE
phvwr phv.inner_udp_valid, TRUE
phvwr phv.udp_valid, FALSE
phvwr phv.ipv4_valid, FALSE
phvwr.e phv.nat_update_inner_checksum, k.nat_update_checksum
phvwr phv.nat_update_checksum, FALSE

ACTION tunnel_encap_update_inner.encap_inner_ipv4_tcp_rewrite:
phvwr phv[inner_ipv4_version...inner_ipv4_dstAddr], k[ipv4_version...ipv4_dstAddr]
phvwr phv.l3_payload_length, k.ipv4_totallen;
phvwr phv.tunnel_inner_ip_proto, IP_PROTOCOLS_IPV4
phvwr phv.inner_ipv4_valid, TRUE
phvwr phv.ipv4_valid, FALSE
phvwr.e phv.nat_update_inner_checksum, k.nat_update_checksum
phvwr phv.nat_update_checksum, FALSE

ACTION tunnel_encap_update_inner.encap_inner_ipv4_icmp_rewrite:
ACTION tunnel_encap_update_inner.encap_inner_ipv4_unknown_rewrite:
struct d {
}
phvwr phv[inner_ipv4_version...inner_ipv4_dstAddr], k[ipv4_version...ipv4_dstAddr]
phvwr phv.inner_ipv4_valid, TRUE
phvwr phv.ipv4_valid, FALSE
phvwr.e phv.l3_payload_length, k.ipv4_totallen;
phvwr phv.tunnel_inner_ip_proto, IP_PROTOCOLS_IPV4

Key format 2 (IPv6):
struct k {
  phv.tcp_valid : 1;
  phv.udp_valid : 1;
  phv.icmp_valid : 1;
  phv.nat_update_checksum : 1;

  // 320 bits
  ipv6_version : 4;
  ipv6_trafficClass : 8;
  ipv6_flowLabel : 20;
  ipv6_payloadLen : 16;
  ipv6_nextHdr : 8;
  ipv6_hopLimit : 8;
  ipv6_srcAddr : 128;
  ipv6_dstAddr : 128;

  // 64 bits
  udp_srcPort : 16;
  udp_dstPort : 16;
  udp_len : 16;
  udp_checksum : 16;
}

ACTION tunnel_encap_update_inner.encap_inner_ipv6_udp_rewrite:
phvwr phv[inner_ipv6_version...inner_ipv6_dstAddr], k[ipv6_version...ipv6_dstAddr]
phvwr phv[inner_udp_srcPort...inner_udp_checksum], k[udp_srcPort...udp_checksum]
phvwr phv.inner_ipv6_valid, TRUE
phvwr phv.inner_udp_valid, TRUE
phvwr phv.ipv6_valid, FALSE
phvwr phv.udp_valid, FALSE
add r1, k.ipv6_payloadlen, 40
phvwr.e phv.l3_payload_length, r1
phvwr phv.tunnel_inner_ip_proto, IP_PROTOCOLS_IPV6

ACTION tunnel_encap_update_inner.encap_inner_ipv6_tcp_rewrite:
ACTION tunnel_encap_update_inner.encap_inner_ipv6_icmp_rewrite:
ACTION tunnel_encap_update_inner.encap_inner_ipv6_unknown_rewrite:
phvwr phv[inner_ipv6_version...inner_ipv6_dstAddr], k[ipv6_version...ipv6_dstAddr]
phvwr phv.inner_ipv6_valid, TRUE
phvwr phv.ipv6_valid, FALSE
add r1, k.ipv6_payloadlen, 40
phvwr.e phv.l3_payload_length, r1
phvwr phv.tunnel_inner_ip_proto, IP_PROTOCOLS_IPV6

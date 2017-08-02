/******************************************************************************/
TABLE : validate_flow_key
/******************************************************************************/
key format : 1
tunnel_terminate == false
phv.lkp_type == FLOW_KEY_LOOKUP_TYPE_MAC
struct k {
  phv.lkp_type : 4;
  phv.ethernet_srcAddr : 48;
  phv.ethernet.dstAddr : 48;
}

key format : 2
tunnel_terminate == false
phv.lkp_type == FLOW_KEY_LOOKUP_TYPE_IPV4
struct k {
  phv.lkp_type : 4;
  phv.ipv4_srcAddr : 32;
  phv.ipv4_dstAddr : 32;
  phv.flow_ttl : 8;
  phv.flow_version : 4;
  phv.flop_ipv4_hlen : 4;
  phv.flow_ipv4_flags : 3;
  phv.flow_fragoffset : 13;
  phv.tcp_flags : 8;
  phv.l3_payload_length : 16;
  phv.tcp_xmas_tree_packet_drop_action : 1;
}

key format : 3
tunnel_terminate == false
phv.lkp_type == FLOW_KEY_LOOKUP_TYPE_IPV6
struct k {
  phv.lkp_type : 4;
  phv.ipv6_srcAddr : 128;
  phv.ipv6_dstAddr : 128;
  phv.flow_ttl : 8;
  phv.flow_version : 4;
  phv.tcp_flags : 8;
  phv.tcp_xmas_tree_packet_drop_action : 1;
}

key format : 4
tunnel_terminate == true
phv.lkp_type == FLOW_KEY_LOOKUP_TYPE_MAC
struct k {
  phv.lkp_type : 4;
  phv.inner_ethernet_srcAddr : 48;
  phv.inner_ethernet.dstAddr : 48;
}

key format : 5
tunnel_terminate == true
phv.lkp_type == FLOW_KEY_LOOKUP_TYPE_IPV4
struct k {
  phv.lkp_type : 4;
  phv.inner_ipv4_srcAddr : 32;
  phv.inner_ipv4_dstAddr : 32;
  phv.flow_ttl : 8;
  phv.flow_version : 4;
  phv.flop_ipv4_hlen : 4;
  phv.flow_ipv4_flags : 3;
  phv.flow_fragoffset : 13;
  phv.inner_tcp_flags : 8;
  phv.l3_payload_length : 16;
  phv.tcp_xmas_tree_packet_drop_action : 1;
}

key format : 6
tunnel_terminate == true
phv.lkp_type == FLOW_KEY_LOOKUP_TYPE_IPV6
struct k {
  phv.lkp_type : 4;
  phv.inner_ipv6_srcAddr : 128;
  phv.inner_ipv6_dstAddr : 128;
  phv.flow_ttl : 8;
  phv.flow_version : 4;
  phv.tcp_flags : 8;
  phv.tcp_xmas_tree_packet_drop_action : 1;
}

ACTION validate_flow_key.malformed_flow_key:
struct d {
}
phvwr.e phv.control_malformed_key_drop, TRUE
nop

ACTION validate_flow_key.ipv4_fragment_last_pkt
struct d {
}
sub r1, k.l3_payload_length, k.flow_ipv4_hlen, 2
add r1, r1, k.flow_fragoffset, 3
sle.e c1, r1, 0xFFFF
phvwr.!c1 phv.ipv4_ping_of_death_drop, TRUE

ACTION validate_flow_key.ipv4_fragment_too_small
struct d {
}
slt.e c1, k.l3_payload_length, 576
phvwr.c1 phv.control_ipv4_fragment_too_small, TRUE

ACTION validate_flow_key.tcp_xmas_tree_packet
struct d {
}
seq.e c1 k.tcp_xmas_tree_packet_drop_action, TRUE
phvwr.c1 phv.tcp_xmas_tree_packet_drop, TRUE

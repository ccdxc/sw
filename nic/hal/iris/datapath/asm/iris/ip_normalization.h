// c7 will retain the tunnel_terminate state throughout this function
// c6 - IPv6
// c5 - IPv4
ip_normalization:
  // First do all IP normalizaiton checks here optimally before we
  // proceed to checking each knob.
  // we can use bbeq here to branch if we spare a bit for V4 and V6. But
  // that might increase the lkp_type beyond 4 bits. So not chaning it for now.
  // Good Packet start
  seq         c7, k.tunnel_metadata_tunnel_terminate, 1
  seq         c5, k.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV4
  seq         c6, k.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV6
  b.c5        lb_ipv4_normalizaiton_optimal
  seq         c2, k.vlan_tag_valid, 1
  b.c6        lb_ipv6_normalization_optimal
  seq         c1, k.l3_metadata_ip_option_seen, TRUE
  jr          r7

// c2 has vlan_tag_valid
lb_ipv4_normalizaiton_optimal:
  seq         c3, k.flow_lkp_metadata_ip_ttl, r0
  cmov        r1, c7, k.inner_ipv4_version, k.ipv4_version
  sne.!c3     c3, r1, 4
  sne         c4, k.ipv4_version, 4
  andcf       c4, [c7]
  bcf         [c3|c4], lb_malformed_packet
  smneb       c1, k.flow_lkp_metadata_ipv4_flags, (IP_FLAGS_RSVD_MASK | IP_FLAGS_DF_MASK), 0
  slt.!c1     c1, 5, k.flow_lkp_metadata_ipv4_hlen
  add.c2      r1, k.ipv4_totalLen, 18 // c2 has vlan_tag_valid to TRUE
  add.!c2     r1, k.ipv4_totalLen, 14
  slt.!c1     c1, r1, k.capri_p4_intrinsic_packet_len
  seq         c3, k.control_metadata_uplink, FALSE
  sne.c3      c3, k.l4_metadata_ip_normalize_ttl, r0
  sne.c3      c3, k.flow_lkp_metadata_ip_ttl, k.l4_metadata_ip_normalize_ttl
  jrcf        ![c1 | c3], r7
  bcf         [c1 | c3], lb_ipv4_normalizaiton // bad packet
  nop

// c1 has ipv6_options_blob_valid == TRUE
// c2 has vlan_tag_valid
lb_ipv6_normalization_optimal:
  seq         c3, k.flow_lkp_metadata_ip_ttl, r0
  cmov        r1, c7, k.inner_ipv6_version, k.ipv6_version
  sne.!c3     c3, r1, 6
  sne         c4, k.ipv4_version, 4
  andcf       c4, [c7]
  bcf         [c3|c4], lb_malformed_packet
  add.c2      r1, k.ipv6_payloadLen, 58 // c2 has vlan_tag_valid to TRUE
  add.!c2     r1, k.ipv6_payloadLen, 54
  slt.!c1     c1, r1, k.capri_p4_intrinsic_packet_len
  seq         c3, k.control_metadata_uplink, FALSE
  sne.c3      c3, k.l4_metadata_ip_normalize_ttl, r0
  sne.c3      c3, k.flow_lkp_metadata_ip_ttl, k.l4_metadata_ip_normalize_ttl
  jrcf        ![c1 | c3], r7
  jr.!c1      r7
  bcf         [c1 | c3], lb_ipv6_normalization // bad packet
  nop
  // Good packet end

lb_ipv4_normalizaiton:
  // Will fall through non-optimal logic for bad packet cases
  // c5 - v4 packet, c6 - v6 packet
  seq         c4, k.l4_metadata_ip_rsvd_flags_action, NORMALIZATION_ACTION_ALLOW
  b.c4        lb_ipv4_norm_df_bit
  seq         c4, k.l4_metadata_ip_df_action, NORMALIZATION_ACTION_ALLOW
  smeqb       c1, k.flow_lkp_metadata_ipv4_flags, IP_FLAGS_RSVD_MASK, IP_FLAGS_RSVD_MASK
  b.!c1       lb_ipv4_norm_df_bit
  seq         c1, k.l4_metadata_ip_rsvd_flags_action, NORMALIZATION_ACTION_DROP
  phvwr.c1.e  p.control_metadata_drop_reason[DROP_IP_NORMALIZATION], 1
  phvwr.c1    p.capri_intrinsic_drop, 1
  // Edit Case. Need to check whether to update inner or outer
  phvwrmi.c7  p.inner_ipv4_flags, 0, IP_FLAGS_RSVD_MASK
  phvwrmi.!c7 p.ipv4_flags, 0, IP_FLAGS_RSVD_MASK
  // Assumption is if we terminated the tunnel and looking inner then we will move all
  // inner packets to outer, so we only need to tell checksum engine to update outer only.
  phvwr       p.control_metadata_checksum_ctl[CHECKSUM_CTL_IP_CHECKSUM], TRUE

lb_ipv4_norm_df_bit:
  b.c4        lb_ipv4_norm_options
  seq         c4, k.l4_metadata_ip_options_action, NORMALIZATION_ACTION_ALLOW
  smeqb       c1, k.flow_lkp_metadata_ipv4_flags, IP_FLAGS_DF_MASK, IP_FLAGS_DF_MASK
  b.!c1       lb_ipv4_norm_options
  seq         c1, k.l4_metadata_ip_df_action, NORMALIZATION_ACTION_DROP
  phvwr.c1.e  p.control_metadata_drop_reason[DROP_IP_NORMALIZATION], 1
  phvwr.c1    p.capri_intrinsic_drop, 1
  // Edit Case. Need to check whether to update inner or outer
  phvwrmi.!c7 p.ipv4_flags, 0, IP_FLAGS_DF_MASK
  phvwrmi.c7  p.inner_ipv4_flags, 0, IP_FLAGS_DF_MASK
  // Assumption is if we terminated the tunnel and looking inner then we will move all
  // inner packets to outer, so we only need to tell checksum engine to update outer only.
  phvwr       p.control_metadata_checksum_ctl[CHECKSUM_CTL_IP_CHECKSUM], TRUE

lb_ipv4_norm_options:
  b.c4        lb_ipv4_norm_invalid_length
  seq         c4, k.l4_metadata_ip_invalid_len_action, NORMALIZATION_ACTION_ALLOW
  slt         c1, 5, k.flow_lkp_metadata_ipv4_hlen
  b.!c1       lb_ipv4_norm_invalid_length
  seq         c1, k.l4_metadata_ip_options_action, NORMALIZATION_ACTION_DROP
  phvwr.c1.e  p.control_metadata_drop_reason[DROP_IP_NORMALIZATION], 1
  phvwr.c1    p.capri_intrinsic_drop, 1
  // Edit Case. Need to check whether to update inner or outer
  // We also need to update
  // 1. IPv4 hlen in packet
  // 2. IPv4 Total Len in packet
  // 3. IP header checksum update, meaning change the header valid bit
  //    to the one which will trigger checksum update
  // 4. control_metadata.packet_len needs to be reduced.
  b.c7        lb_ipv4_norm_options_tunnel_terminate
  phvwr.!c7   p.ipv4_options_blob_valid, 0
  phvwr       p.ipv4_ihl, 5
  add         r1, r0, k.flow_lkp_metadata_ipv4_hlen, 2
  sub         r1, r1, 20 // Option length
  sub         r2, k.ipv4_totalLen, r1
  phvwr       p.ipv4_totalLen, r2
  sub         r2, k.capri_p4_intrinsic_packet_len, r1
  phvwr       p.capri_p4_intrinsic_packet_len, r2
  b           lb_ipv4_norm_invalid_length
  phvwr       p.control_metadata_checksum_ctl[CHECKSUM_CTL_IP_CHECKSUM], TRUE


lb_ipv4_norm_options_tunnel_terminate:
  phvwr       p.inner_ipv4_options_blob_valid, 0
  phvwr       p.inner_ipv4_ihl, 5
  add         r1, r0, k.flow_lkp_metadata_ipv4_hlen, 2
  sub         r1, r1, 20 // Option length
  sub         r2, k.inner_ipv4_totalLen, r1
  phvwr       p.inner_ipv4_totalLen, r2
  sub         r2, k.udp_len, r1
  phvwr       p.udp_len, r2
  sub         r2, k.ipv4_totalLen, r1
  phvwr       p.ipv4_totalLen, r2
  sub         r2, k.capri_p4_intrinsic_packet_len, r1
  phvwr       p.capri_p4_intrinsic_packet_len, r2
  // Assumption is if we terminated the tunnel and looking inner then we will move all
  // inner packets to outer, so we only need to tell checksum engine to update outer only.
  phvwr       p.control_metadata_checksum_ctl[CHECKSUM_CTL_IP_CHECKSUM], TRUE


// Here we normalize the invalid length based on outer IP total len
lb_ipv4_norm_invalid_length:
  b.c4        lb_ipv4_norm_ttl
  seq         c4, k.l4_metadata_ip_normalize_ttl, 0
  seq         c1, k.vlan_tag_valid, 1
  add.c1      r1, k.ipv4_totalLen, 18
  add.!c1     r1, k.ipv4_totalLen, 14
  slt         c2, r1, k.capri_p4_intrinsic_packet_len
  b.!c2       lb_ipv4_norm_ttl
  seq         c2, k.l4_metadata_ip_invalid_len_action, NORMALIZATION_ACTION_DROP
  phvwr.c2.e  p.control_metadata_drop_reason[DROP_IP_NORMALIZATION], 1
  phvwr.c2    p.capri_intrinsic_drop, 1
  // Edit action
  // To edit the packet, we need to know what is the current payload length after
  // the parsed headers so that we can truncate the packet by the difference of
  // (capri_p4_intrinsic_packet_len - (ipv4.totalLen + Ether header + VLAN (if exists)))
  // Without this we need to have a knowledge of all headers that the parser
  // is parsing and then do the calculation so that we update the
  // capri_deparser_len_trunc_pkt_len.
  // When editing we need to make sure we don't truncate lower than 64 byte packet
  seq         c2, k.control_metadata_parse_tcp_option_error, TRUE
  sle.!c2     c2, k.capri_p4_intrinsic_packet_len, MIN_ETHER_FRAME_LEN
  b.c2        lb_ipv4_norm_ttl
  // Calculate the total parsed packet length from ethernet header onwards.
  // If we parsed more than the packet length we will not truncate the packet.
  // Reason being we don't what all headers are parsed and marking them all
  // invalid might not be striaght forward.
  sub         r2, k.control_metadata_parser_payload_offset, k.control_metadata_parser_outer_eth_offset
  seq         c2, k.p4plus_to_p4_insert_vlan_tag, 1
  add.c2      r2, r2, 4
  // This case should be ideally catched by parser "packet_len_check" pragma.
  sle         c2, k.capri_p4_intrinsic_packet_len, r1
  b.c2        lb_ipv4_norm_ttl
  // Now calculate what is the value to write to capri_deparser_len_trunc_pkt_len.
  sub         r3, r1, r2
  phvwr       p.capri_deparser_len_trunc_pkt_len, r3
  phvwr       p.capri_deparser_len_trunc, 1
  phvwr       p.capri_p4_intrinsic_packet_len, r1
  b           lb_ipv4_norm_ttl
  phvwr       p.capri_intrinsic_payload, 0

#if 0
 For invalid length we will only normalize based on outer IP total length.
 If we really need to do based on tunnel termination for inner then the
 optimal code which checks for bad packets also need to do this check and
 in input_mapping_native/tunnel we need to normalize the ipv4 total length
 to a common field.
// c1 - Has vlan_valid flag
lb_ipv4_norm_invalid_length_tunnel_terminate:
  add.c1      r1, k.ipv4_ihl, 34    // 18 (eth+vlan) + 8 (udp) + 8 (vxlan)
  add.!c1     r1, k.ipv4_ihl, 30    // 14 (eth) + 8 (udp) + 8 (vxlan)
  add         r1, r1, k.inner_ipv4_totalLen
  slt         c1, r1, k.capri_p4_intrinsic_packet_len
  b.!c1       lb_ipv4_norm_ttl
  seq         c1, k.l4_metadata_ip_invalid_len_action, NORMALIZATION_ACTION_DROP
  phvwr.c1.e  p.control_metadata_drop_reason[DROP_IP_NORMALIZATION], 1
  phvwr.c1    p.capri_intrinsic_drop, 1
  // Edit action
  // To edit the packet, we need to know what is the current payload length after
  // the parsed headers so that we can truncate the packet by the difference of
  // (capri_p4_intrinsic_packet_len - (ipv4.totalLen + Ether header + VLAN (if exists)))
  // Without this we need to have a knowledge of all headers that the parser
  // is parsing and then do the calculation so that we update the
  // capri_deparser_len_trunc_pkt_len.
  // When editing we need to make sure we don't truncate lower than 64 byte packet
  // For VxLAN terminated packets this is not a isue.
  // Calculate the total parsed packet length from ethernet header onwards.
  // If we parsed more than the packet length we will not truncate the packet.
  // Reason being we don't what all headers are parsed and marking them all
  // invalid might not be striaght forward.
  sub         r2, k.control_metadata_parser_payload_offset, k.control_metadata_parser_inner_eth_offset
  // This case should be ideally catched by parser "packet_len_check" pragma.
  sle         c2, k.capri_p4_intrinsic_packet_len, r1
  b.c2        lb_ipv4_norm_ttl
  // Now calculate what is the value to write to capri_deparser_len_trunc_pkt_len.
  sub         r3, r1, r2
  phvwr       p.capri_deparser_len_trunc_pkt_len, r3
  phvwr       p.capri_deparser_len_trunc, 1
  b           lb_ipv4_norm_ttl
  phvwr       p.capri_intrinsic_payload, 0

#endif /* 0 */

lb_ipv4_norm_ttl:
  jr.c4       r7
  seq         c1, k.control_metadata_uplink, FALSE
  sne.c1      c1, k.flow_lkp_metadata_ip_ttl, k.l4_metadata_ip_normalize_ttl
  jr.!c1      r7
  nop
  // We are here means we have to edit the packet based on tunnel termination
  // There is no DROP option for ttl normalization.
  phvwr.c7    p.inner_ipv4_ttl, k.l4_metadata_ip_normalize_ttl
  phvwr.!c7   p.ipv4_ttl, k.l4_metadata_ip_normalize_ttl
  phvwr       p.control_metadata_checksum_ctl[CHECKSUM_CTL_IP_CHECKSUM], TRUE
  jr          r7
  nop


lb_ipv6_normalization:
  seq         c4, k.l4_metadata_ip_options_action, NORMALIZATION_ACTION_ALLOW
  b.c4        lb_ipv6_norm_invalid_length
  seq         c4, k.l4_metadata_ip_invalid_len_action, NORMALIZATION_ACTION_ALLOW
  seq         c1, k.l3_metadata_ip_option_seen, 1
  b.!c1       lb_ipv6_norm_invalid_length
  seq         c1, k.l4_metadata_ip_options_action, NORMALIZATION_ACTION_DROP
  phvwr.c1.e  p.control_metadata_drop_reason[DROP_IP_NORMALIZATION], 1
  phvwr.c1    p.capri_intrinsic_drop, 1
  // Edit Case. Need to check whether to update inner or outer
  // We also need to update
  // 1. First mark the ipv6 option blob as not valid.
  // 2. Update the nxt_hdr in main ipv6 hdr to ulp
  // 3. IPv6 payload length in packet
  // 4. capri_p4_intrinsic.packet_len needs to be reduced
  b.c7        lb_ipv6_norm_options_tunnel_terminate
  phvwr.!c7   p.ipv6_options_blob_valid, 0
  phvwr       p.ipv6_nextHdr, k.l3_metadata_ipv6_ulp
  sub         r1, k.capri_p4_intrinsic_packet_len, k.l3_metadata_ipv6_options_len
  phvwr       p.capri_p4_intrinsic_packet_len, r1
  sub         r1, k.ipv6_payloadLen, k.l3_metadata_ipv6_options_len
  b           lb_ipv6_norm_invalid_length
  phvwr       p.ipv6_payloadLen, r1

lb_ipv6_norm_options_tunnel_terminate:
  phvwr       p.inner_ipv6_options_blob_valid, 0
  phvwr       p.inner_ipv6_nextHdr, k.l3_metadata_inner_ipv6_ulp
  sub         r1, k.capri_p4_intrinsic_packet_len, k.l3_metadata_inner_ipv6_options_len
  phvwr       p.capri_p4_intrinsic_packet_len, r1
  sub         r1, k.inner_ipv6_payloadLen, k.l3_metadata_inner_ipv6_options_len
  phvwr       p.inner_ipv6_payloadLen, r1
  sub         r1, k.udp_len, k.l3_metadata_inner_ipv6_options_len
  phvwr       p.udp_len, r1
  sub         r1, k.ipv4_totalLen, k.l3_metadata_inner_ipv6_options_len
  phvwr       p.ipv4_totalLen, r1

// Here we normalize the invalid length based on outer IP total len
lb_ipv6_norm_invalid_length:
  b.c4        lb_ipv6_norm_hop_limit
  seq         c4, k.l4_metadata_ip_normalize_ttl, 0
  seq         c1, k.vlan_tag_valid, 1
  add.c1      r1, k.ipv6_payloadLen, 58
  add.!c1     r1, k.ipv6_payloadLen, 54
  slt         c2, r1, k.capri_p4_intrinsic_packet_len
  b.!c2       lb_ipv6_norm_hop_limit
  seq         c2, k.l4_metadata_ip_invalid_len_action, NORMALIZATION_ACTION_DROP
  phvwr.c2.e  p.control_metadata_drop_reason[DROP_IP_NORMALIZATION], 1
  phvwr.c2    p.capri_intrinsic_drop, 1
  // Edit action
  // To edit the packet, we need to know what is the current payload length after
  // the parsed headers so that we can truncate the packet by the difference of
  // (capri_p4_intrinsic_packet_len - (ipv4.totalLen + Ether header + VLAN (if exists)))
  // Without this we need to have a knowledge of all headers that the parser
  // is parsing and then do the calculation so that we update the
  // capri_deparser_len_trunc_pkt_len.
  // When editing we need to make sure we don't truncate lower than 64 byte packet
  sle         c2, k.capri_p4_intrinsic_packet_len, MIN_ETHER_FRAME_LEN
  b.c2        lb_ipv6_norm_hop_limit
  // Calculate the total parsed packet length from ethernet header onwards.
  // If we parsed more than the packet length we will not truncate the packet.
  // Reason being we don't what all headers are parsed and marking them all
  // invalid might not be striaght forward.
  sub         r2, k.control_metadata_parser_payload_offset, k.control_metadata_parser_outer_eth_offset
  // This case should be ideally catched by parser "packet_len_check" pragma.
  sle         c2, k.capri_p4_intrinsic_packet_len, r1
  b.c2        lb_ipv6_norm_hop_limit
  // Now calculate what is the value to write to capri_deparser_len_trunc_pkt_len.
  sub         r3, r1, r2
  phvwr       p.capri_deparser_len_trunc_pkt_len, r3
  phvwr       p.capri_deparser_len_trunc, 1
  phvwr       p.capri_p4_intrinsic_packet_len, r1
  b           lb_ipv6_norm_hop_limit
  phvwr       p.capri_intrinsic_payload, 0

lb_ipv6_norm_hop_limit:
  jr.c4       r7
  seq         c1, k.control_metadata_uplink, FALSE
  sne.c1      c1, k.flow_lkp_metadata_ip_ttl, k.l4_metadata_ip_normalize_ttl
  jr.!c1      r7
  nop
  // We are here means we have to edit the packet based on tunnel termination
  // There is no DROP option for ttl normalization.
  phvwr.c7    p.inner_ipv6_hopLimit, k.l4_metadata_ip_normalize_ttl
  phvwr.!c7   p.ipv6_hopLimit, k.l4_metadata_ip_normalize_ttl
  phvwr       p.control_metadata_checksum_ctl[CHECKSUM_CTL_IP_CHECKSUM], TRUE
  jr          r7
  nop

lb_malformed_packet:
  phvwr.e     p.control_metadata_drop_reason[DROP_MALFORMED_PKT], 1
  phvwr       p.capri_intrinsic_drop, 1

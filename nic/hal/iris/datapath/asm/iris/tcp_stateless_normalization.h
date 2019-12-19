// c2 - C2 is only used to check the next configuration knob in the delay
//      slot. So don't use anywhere in the other logic.
// General order of execution is
// 1. Check for knob Allow
// 2. If knob is not allow then check for the error condition.
// 3. If there is no error in packet. Jump to next knob.
// 4. If packet has the error then check if knob is drop and drop it.
// 5. If its not drop the only option is Edit, edit the packet and
//    jump to next knob.

// c7 - smeqb       c7, k.tcp_flags, TCP_FLAG_URG, TCP_FLAG_URG
// c6 - smneb       c6, k.tcp_flags, TCP_FLAG_SYN, TCP_FLAG_SYN
// c5 - seq         c5, k.tcp_urgentPtr, r0
// c4 - seq         c4, k.l4_metadata_tcp_data_len, r0

tcp_stateless_normalization:
  smeqb       c7, k.tcp_flags, TCP_FLAG_URG, TCP_FLAG_URG
  smneb       c6, k.tcp_flags, TCP_FLAG_SYN, TCP_FLAG_SYN
  seq         c5, k.tcp_urgentPtr, r0
  seq         c4, k.l4_metadata_tcp_data_len, r0

  sne         c1, k.tcp_res, r0  // Reserved Flag set


  sne         c2, k.{tcp_option_sack_perm_valid...tcp_option_mss_valid}, r0
#if 0
  seq         c2, k.tcp_option_mss_valid, TRUE
  seq         c3, k.tcp_option_ws_valid, TRUE
  setcf       c2, [c2 | c3]
#endif /* 0 */

  setcf.!c1   c1, [c2 & c6]

  // Three checks covered so far and aggregated result in c1
  // 1. reservred flags, 2. mss with syn 3. ws with syn
  // c2, c3 free to use.
  setcf.!c1   c1, [!c7 & !c5]  // urg flag not set and urg ptr not zero

  setcf.!c1   c1, [c7 & !c5 & c4]  // urg payload missing

  setcf.!c1   c1, [c7 & c5]   // urg flag set and urg ptr is zero

  smeqb       c2, k.tcp_flags, TCP_FLAG_RST, TCP_FLAG_RST

  setcf.!c1   c1, [c2 & !c4] // rst flag set and data len is not zero

#if 0
  smeqb       c2, k.tcp_flags, TCP_FLAG_SYN|TCP_FLAG_RST, TCP_FLAG_SYN|TCP_FLAG_RST
  smeqb       c3, k.tcp_flags, TCP_FLAG_SYN|TCP_FLAG_FIN, TCP_FLAG_SYN|TCP_FLAG_FIN
  setcf.!c1   c1, [c2 | c3]  // invalid flags syn+rst or syn+fin
#else /* 0 */
  add         r1, k.tcp_flags, r0
  indexb      r2, r1, [TCP_FLAG_SYN|TCP_FLAG_RST, TCP_FLAG_SYN|TCP_FLAG_FIN, 0xFF, 0x0], 0
  sle.s.!c1   c1, r0, r2
#endif /* 0 */

  smeqb       c2, k.tcp_flags, TCP_FLAG_ACK, 0x0

  setcf.!c1   c1, [c2 & c6]   // ACK is not set and SYN is not set

  seq.c2      c2, k.tcp_option_timestamp_valid, TRUE // ACK not set and timestamp valid
  sne.c2      c2, k.tcp_option_timestamp_prev_echo_ts, r0 // ACK not set and timestamp valid and echo_ts non-zero

  setcf.!c1   c1, [c2 | !c6] // !c6 is for SYN packets to do tcp_mss normalizaiton

  nop.!c1.e
  b.c1        lb_tcp_rsvd_flags    // We hit a bad packet.

#if 0
  sne         c1, k.tcp_res, r0  // Reserved Flag set

  // unexpected mss
  smneb       c3, k.tcp_flags, TCP_FLAG_SYN, TCP_FLAG_SYN
  seq         c4, k.tcp_option_mss_valid, TRUE

  // unexpected win scale
  smneb       c3, k.tcp_flags, TCP_FLAG_SYN, TCP_FLAG_SYN
  seq         c4, k.tcp_option_ws_valid, TRUE

  // unexpected sack_perm scale
  smneb       c3, k.tcp_flags, TCP_FLAG_SYN, TCP_FLAG_SYN
  seq         c4, k.tcp_option_sack_perm_valid, TRUE

  // Urg flag is not set
  smneb       c3, k.tcp_flags, TCP_FLAG_URG, TCP_FLAG_URG
  sne         c4, k.tcp_urgentPtr, r0

  // urg payload is missing
  smeqb       c3, k.tcp_flags, TCP_FLAG_URG, TCP_FLAG_URG
  sne         c4, k.tcp_urgentPtr, r0
  seq         c5, k.l4_metadata_tcp_data_len, r0

  // urg ptr not set
  smeqb       c3, k.tcp_flags, TCP_FLAG_URG, TCP_FLAG_URG
  seq         c4, k.tcp_urgentPtr, r0

  // rst with data
  smeqb       c3, k.tcp_flags, TCP_FLAG_RST, TCP_FLAG_RST
  sne         c4, k.l4_metadata_tcp_data_len, r0

  // invalid flags
  smeqb       c3, k.tcp_flags, TCP_FLAG_SYN|TCP_FLAG_RST, TCP_FLAG_SYN|TCP_FLAG_RST
  smeqb       c4, k.tcp_flags, TCP_FLAG_SYN|TCP_FLAG_FIN, TCP_FLAG_SYN|TCP_FLAG_FIN

  // no syn no ack
  smeqb       c3, k.tcp_flags, TCP_FLAG_SYN, 0x0
  smeqb       c4, k.tcp_flags, TCP_FLAG_ACK, 0x0

  // unexpected echo_ts
  smeqb       c3, k.tcp_flags, TCP_FLAG_ACK, 0x0
  seq.c3      c3, k.tcp_option_timestamp_valid, TRUE
  sne.c3      c3, k.tcp_option_timestamp_prev_echo_ts, r0
#endif /* 0 */

lb_tcp_rsvd_flags:
  seq         c2, k.l4_metadata_tcp_rsvd_flags_action, NORMALIZATION_ACTION_ALLOW
  b.c2        lb_tcp_unexpected_mss
  seq         c2, k.l4_metadata_tcp_unexpected_mss_action, NORMALIZATION_ACTION_ALLOW
  sne         c3, k.tcp_res, r0
  b.!c3       lb_tcp_unexpected_mss
  seq         c4, k.l4_metadata_tcp_rsvd_flags_action, \
                NORMALIZATION_ACTION_DROP
  phvwr.c4.e  p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c4    p.capri_intrinsic_drop, 1
  // If not Allow/Drop then its EDIT
  phvwr       p.tcp_res, 0
  phvwr       p.control_metadata_checksum_ctl[CHECKSUM_CTL_L4_CHECKSUM], TRUE


// C2 has lb_tcp_unexpected_mss_action == ALLOW
lb_tcp_unexpected_mss:
  // or          r1, k.l4_metadata_tcp_unexpected_win_scale_action_s1_e1, \
  //                 k.l4_metadata_tcp_unexpected_win_scale_action_s0_e0, 1
  b.c2        lb_tcp_unexpected_win_scale
  // seq         c2, r1, NORMALIZATION_ACTION_ALLOW
  seq         c2, k.l4_metadata_tcp_unexpected_win_scale_action, NORMALIZATION_ACTION_ALLOW
  smneb       c3, k.tcp_flags, TCP_FLAG_SYN, TCP_FLAG_SYN
  seq         c4, k.tcp_option_mss_valid, TRUE
  bcf         ![c3 & c4], lb_tcp_unexpected_win_scale
  seq         c3, k.l4_metadata_tcp_unexpected_mss_action, \
                NORMALIZATION_ACTION_DROP
  phvwr.c3.e  p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c3    p.capri_intrinsic_drop, 1
  // Edit option: Remove the mss option
  phvwr       p.tcp_option_mss_valid, FALSE
  phvwr       p.tcp_options_blob_valid, FALSE


lb_tcp_unexpected_win_scale:
  b.c2        lb_tcp_unexpected_sack_perm
  seq         c2, k.l4_metadata_tcp_unexpected_sack_perm_action, \
                     NORMALIZATION_ACTION_ALLOW
  smneb       c3, k.tcp_flags, TCP_FLAG_SYN, TCP_FLAG_SYN
  seq         c4, k.tcp_option_ws_valid, TRUE
  bcf         ![c3 & c4], lb_tcp_unexpected_sack_perm
  // r1 is updated in lb_tcp_unexpected_mss
  seq         c3, r1, NORMALIZATION_ACTION_DROP
  phvwr.c3.e  p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c3    p.capri_intrinsic_drop, 1
  // Edit option: Remove the mss option
  phvwr       p.tcp_option_ws_valid, FALSE
  phvwr       p.tcp_options_blob_valid, FALSE

lb_tcp_unexpected_sack_perm:
  b.c2        lb_tcp_urg_flag_not_set
  seq         c2, k.l4_metadata_tcp_urg_flag_not_set_action, \
                     NORMALIZATION_ACTION_ALLOW
  smneb       c3, k.tcp_flags, TCP_FLAG_SYN, TCP_FLAG_SYN
  seq         c4, k.tcp_option_sack_perm_valid, TRUE
  bcf         ![c3 & c4], lb_tcp_urg_flag_not_set
  seq         c3, k.l4_metadata_tcp_unexpected_sack_perm_action, \
                     NORMALIZATION_ACTION_DROP
  phvwr.c3.e  p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c3    p.capri_intrinsic_drop, 1
  // Edit option: Remove the mss option
  phvwr       p.tcp_option_sack_perm_valid, FALSE
  phvwr       p.tcp_options_blob_valid, FALSE


lb_tcp_urg_flag_not_set:
  b.c2        lb_tcp_urg_payload_missing
  // or          r1, k.l4_metadata_tcp_urg_payload_missing_action_s1_e1, \
  //                 k.l4_metadata_tcp_urg_payload_missing_action_s0_e0, 1
  // seq         c2, r1, NORMALIZATION_ACTION_ALLOW
  seq         c2, k.l4_metadata_tcp_urg_payload_missing_action, \
                     NORMALIZATION_ACTION_ALLOW
  smneb       c3, k.tcp_flags, TCP_FLAG_URG, TCP_FLAG_URG
  sne         c4, k.tcp_urgentPtr, r0
  bcf         ![c3 & c4], lb_tcp_urg_payload_missing
  seq         c3, k.l4_metadata_tcp_urg_flag_not_set_action, \
                     NORMALIZATION_ACTION_DROP
  phvwr.c3.e  p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c3    p.capri_intrinsic_drop, 1
  // If not Allow/Drop then its EDIT
  phvwr       p.tcp_urgentPtr, 0
  phvwr       p.control_metadata_checksum_ctl[CHECKSUM_CTL_L4_CHECKSUM], TRUE

lb_tcp_urg_payload_missing:
  b.c2        lb_tcp_urg_ptr_not_set
  seq         c2, k.l4_metadata_tcp_urg_ptr_not_set_action, \
                     NORMALIZATION_ACTION_ALLOW
  smeqb       c3, k.tcp_flags, TCP_FLAG_URG, TCP_FLAG_URG
  sne         c4, k.tcp_urgentPtr, r0
  seq         c5, k.l4_metadata_tcp_data_len, r0
  bcf         ![c3 & c4 & c5], lb_tcp_urg_ptr_not_set
  // or          r1, k.l4_metadata_tcp_urg_payload_missing_action_s1_e1, \
  //                 k.l4_metadata_tcp_urg_payload_missing_action_s0_e0, 1
  // seq         c3, r1, NORMALIZATION_ACTION_DROP
  seq         c3, k.l4_metadata_tcp_urg_payload_missing_action, \
                     NORMALIZATION_ACTION_DROP
  phvwr.c3.e  p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c3    p.capri_intrinsic_drop, 1
  // If not Allow/Drop then its EDIT
  phvwr       p.tcp_urgentPtr, 0
  phvwrmi     p.tcp_flags, 0x0, TCP_FLAG_URG
  phvwr       p.control_metadata_checksum_ctl[CHECKSUM_CTL_L4_CHECKSUM], TRUE


lb_tcp_urg_ptr_not_set:
  b.c2        lb_tcp_rst_with_data
  seq         c2, k.l4_metadata_tcp_rst_with_data_action, \
                    NORMALIZATION_ACTION_ALLOW
  smeqb       c3, k.tcp_flags, TCP_FLAG_URG, TCP_FLAG_URG
  seq         c4, k.tcp_urgentPtr, r0
  bcf         ![c3 & c4], lb_tcp_rst_with_data
  seq         c3, k.l4_metadata_tcp_urg_ptr_not_set_action, \
                     NORMALIZATION_ACTION_DROP
  phvwr.c3.e  p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c3    p.capri_intrinsic_drop, 1
  // If not Allow/Drop then its EDIT
  phvwrmi     p.tcp_flags, 0x0, TCP_FLAG_URG
  phvwr       p.control_metadata_checksum_ctl[CHECKSUM_CTL_L4_CHECKSUM], TRUE


lb_tcp_rst_with_data:
  b.c2        lb_tcp_invalid_flags
  seq         c2, k.l4_metadata_tcp_invalid_flags_drop, ACT_ALLOW
  smeqb       c3, k.tcp_flags, TCP_FLAG_RST, TCP_FLAG_RST
  sne         c4, k.l4_metadata_tcp_data_len, r0
  bcf         ![c3 & c4], lb_tcp_invalid_flags
  seq         c3, k.l4_metadata_tcp_rst_with_data_action, \
                    NORMALIZATION_ACTION_DROP
  phvwr.c3.e  p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c3    p.capri_intrinsic_drop, 1
  // Edit option - We will strip the packet so that the tcp_data_len is zero
  add          r1, k.l4_metadata_tcp_data_len, r0 // we need to subtract tcp_data_len
  sub          r5, k.capri_p4_intrinsic_packet_len, r1 // r5 = k.capri_p4_intrinsic_packet_len - r1
  phvwr        p.capri_p4_intrinsic_packet_len, r5
  phvwr        p.capri_deparser_len_trunc_pkt_len, r0 // Zero out the tcp payload
  sub          r5, k.ipv4_totalLen, r1   // r5 = k.ipv4_totalLen - r1
  phvwr        p.ipv4_totalLen, r5
  seq          c1, k.tunnel_metadata_tunnel_terminate, TRUE
  sub.c1       r5, k.udp_len, r1   // r5 = k.udp.len - r1
  phvwr.c1     p.udp_len, r5
  sub.c1       r5, k.inner_ipv4_totalLen, r1   // r5 = k.inner_ipv4_totalLen - r1
  phvwr.c1     p.inner_ipv4_totalLen, r5
  // Finally update the tcp_data_len to zero value which will be used by
  // connection tracking code
  phvwr        p.l4_metadata_tcp_data_len, r0
  phvwr        p.capri_intrinsic_payload, 0
  phvwr        p.capri_deparser_len_trunc, 1
  phvwrmi      p.control_metadata_checksum_ctl, CHECKSUM_L3_L4_UPDATE_MASK, CHECKSUM_L3_L4_UPDATE_MASK
  // Edit option: TBD
  // 1. Change the l4_metadata.tcp_data_len to zero
  // 2. Update IP header total len
  // 3. Update the intrinsic header frame size.


lb_tcp_invalid_flags:
  b.c2        lb_tcp_flags_nonsyn_noack
  seq         c2, k.l4_metadata_tcp_flags_nonsyn_noack_drop, ACT_ALLOW
  add         r1, k.tcp_flags, r0
  indexb      r2, r1, [TCP_FLAG_SYN|TCP_FLAG_RST, TCP_FLAG_SYN|TCP_FLAG_FIN, 0xFF, 0x0], 0
  sle.s       c3, r0, r2
  b.!c3       lb_tcp_flags_nonsyn_noack
  seq         c3, k.l4_metadata_tcp_invalid_flags_drop, ACT_DROP
  // Trying to be symmetric with rest of the blocks of code. Ideally
  // The above line can be a nop and we can unconditionally drop the packet
  // only allow and drop are the only options. No edit.
  phvwr.c3.e  p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c3    p.capri_intrinsic_drop, 1


lb_tcp_flags_nonsyn_noack:
  b.c2        lb_tcp_normalize_mss
  seq         c2, k.l4_metadata_tcp_normalize_mss, r0
  smeqb       c3, k.tcp_flags, TCP_FLAG_SYN, 0x0
  smeqb       c4, k.tcp_flags, TCP_FLAG_ACK, 0x0
  bcf         ![c3 & c4], lb_tcp_normalize_mss
  seq         c3, k.l4_metadata_tcp_flags_nonsyn_noack_drop, ACT_DROP
  // Trying to be symmetric with rest of the blocks of code. Ideally
  // The above line can be a nop and we can unconditionally drop the packet
  // only allow and drop are the only options. No edit.
  phvwr.c3.e  p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c3    p.capri_intrinsic_drop, 1

lb_tcp_normalize_mss:
  b.c2        lb_tcp_unexpected_echo_ts
  seq         c2, k.l4_metadata_tcp_unexpected_echo_ts_action, \
                    NORMALIZATION_ACTION_ALLOW
  smeqb       c3, k.tcp_flags, TCP_FLAG_SYN, TCP_FLAG_SYN
  b.!c3       lb_tcp_unexpected_echo_ts
  seq         c4, k.tcp_option_mss_valid, TRUE
  phvwr.c4    p.tcp_option_mss_value, k.l4_metadata_tcp_normalize_mss
  // If mss option was not present then we need to add the option 
  // with the tcp_normalize_mss value
  phvwr.!c4   p.tcp_option_mss_valid, TRUE
  phvwr.!c4   p.tcp_option_mss_optType, 0x2
  phvwr.!c4   p.tcp_option_mss_optLength, 4
  phvwr.!c4   p.tcp_option_mss_value, k.l4_metadata_tcp_normalize_mss
  phvwr       p.tcp_options_blob_valid, FALSE
  

lb_tcp_unexpected_echo_ts:
  nop.c2.e
  smeqb       c3, k.tcp_flags, TCP_FLAG_ACK, 0x0
  seq.c3      c3, k.tcp_option_timestamp_valid, TRUE
  sne.c3      c3, k.tcp_option_timestamp_prev_echo_ts, r0
  nop.!c3.e
  seq         c3, k.l4_metadata_tcp_unexpected_echo_ts_action, \
                    NORMALIZATION_ACTION_DROP
  phvwr.c3.e  p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c3    p.capri_intrinsic_drop, 1
  // If not Allow/Drop then its EDIT
  // when we edit the option we will turn of the blob
  phvwr.e     p.tcp_option_timestamp_prev_echo_ts, r0
  phvwr       p.tcp_options_blob_valid, FALSE

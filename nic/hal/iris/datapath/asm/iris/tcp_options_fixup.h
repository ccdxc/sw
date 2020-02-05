tcp_options_fixup:
  seq         c1, k.tcp_valid, TRUE
  nop.!c1.e
  seq         c1, k.tcp_options_blob_valid, TRUE
  .assert(offsetof(p, tcp_option_eol_valid) - offsetof(p, tcp_option_mss_valid) == 11)
  phvwr.c1.e  p.{tcp_option_eol_valid...tcp_option_mss_valid}, r0
  // We have the case where we can add a TCP MSS option for all SYN
  // Packets even if there were no options present in the incoming
  // packet. So the case where we don't have to go through below code
  // if the tcp dataoffset was 5 is not valid anymore.
  sle         c1, k.tcp_dataOffset, 5
  seq.c1      c1, k.tcp_option_mss_valid, FALSE
  nop.c1.e
  add         r1, r0, r0
  seq         c2, k.tcp_option_timestamp_valid, TRUE
  add.c2      r1, r0, 10
  seq         c2, k.tcp_option_four_sack_valid, TRUE
  add.c2      r1, r0, 34
  seq         c2, k.tcp_option_three_sack_valid, TRUE
  add.c2      r1, r1, 26
  seq         c2, k.tcp_option_two_sack_valid, TRUE
  add.c2      r1, r1, 18
  seq         c2, k.tcp_option_one_sack_valid, TRUE
  add.c2      r1, r1, 10
  seq         c2, k.tcp_option_sack_perm_valid, TRUE
  add.c2      r1, r1, 2
  seq         c2, k.tcp_option_ws_valid, TRUE
  add.c2      r1, r1, 3
  seq         c2, k.tcp_option_mss_valid, TRUE
  add.c2      r1, r1, 4
  .assert(offsetof(p, tcp_option_eol_valid) - offsetof(p, tcp_option_unknown_valid) == 3)
  phvwr       p.{tcp_option_eol_valid...tcp_option_unknown_valid}, r0[3:0]
  mod         r2, r1, 4
  .brbegin
  br           r2[1:0]
  add          r1, r1, 20  // Adding TCP header of 20 bytes
  .brcase      0
  b            lb_padding_done
  nop
  .brcase      1
  phvwr        p.tcp_option_nop_valid, 1
  phvwr        p.tcp_option_nop_optType, 1
  phvwr        p.tcp_option_nop_1_valid, 1
  phvwr        p.tcp_option_nop_1_optType, 1
  phvwr        p.tcp_option_eol_valid, 1
  b            lb_padding_done
  add          r1, r1, 3
  .brcase      2
  phvwr        p.tcp_option_nop_valid, 1
  phvwr        p.tcp_option_nop_optType, 1
  phvwr        p.tcp_option_eol_valid, 1
  b            lb_padding_done
  add          r1, r1, 2
  .brcase      3
  phvwr        p.tcp_option_eol_valid, 1
  b            lb_padding_done
  add          r1, r1, 1
  .brend

lb_padding_done:
  // Check if the new tcp header length crossed the max value
  sle          c1, r1, 60
  phvwr.!c1.e  p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.!c1    p.capri_intrinsic_drop, 1

  // check if the packet lengths need to be fixed from what came in
  // r2 - Packets TCP Header length including options
  // r1 - new TCP Header length including options
  add          r2, r0, k.tcp_dataOffset, 2
  seq          c1, r1, r2
  // Update checksums
  // Assumption is if we terminated the tunnel and looking inner then we will move all
  // inner packets to outer, so we only need to tell checksum engine to update outer only.
  // Only TCP checksum as there is no packet length change which can affect IP Checksum
  phvwr         p.control_metadata_checksum_ctl[CHECKSUM_CTL_L4_CHECKSUM], TRUE
  nop.c1.e
  nop

  // Now we need to update all the packet lengths, It could be add or subtract
  sub          r3, r1, r2

  add          r4, k.capri_p4_intrinsic_packet_len, r3
  phvwr        p.capri_p4_intrinsic_packet_len, r4
  add          r4, k.ipv4_totalLen, r3
  phvwr        p.ipv4_totalLen, r4
  phvwr        p.tcp_dataOffset, r1[5:2]
  seq          c1, k.tunnel_metadata_tunnel_terminate, TRUE
  add.c1       r4, k.udp_len, r3
  phvwr.c1     p.udp_len, r4
  add.c1       r4, k.inner_ipv4_totalLen, r3
  phvwr.c1     p.inner_ipv4_totalLen, r4
  // Update checksums
  // Assumption is if we terminated the tunnel and looking inner then we will move all
  // inner packets to outer, so we only need to tell checksum engine to update outer only.
  phvwrmi      p.control_metadata_checksum_ctl, CHECKSUM_L3_L4_UPDATE_MASK, CHECKSUM_L3_L4_UPDATE_MASK
  nop.e
  nop

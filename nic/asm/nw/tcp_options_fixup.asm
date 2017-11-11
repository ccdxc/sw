#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct tcp_options_fixup_k k;
struct phv_              p;

%%

tcp_options_fixup:
  seq         c1, k.tcp_options_blob_valid, TRUE
  .assert(offsetof(p, tcp_option_eol_valid) - offsetof(p, tcp_option_mss_valid) == 11)
  phvwr.c1.e  p.{tcp_option_eol_valid...tcp_option_mss_valid}, r0
  sle         c1, k.tcp_dataOffset, 5
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
  phvwr        p.tcp_option_nop_1_valid, 1
  phvwr        p.tcp_option_eol_valid, 1
  b            lb_padding_done
  add          r1, r1, 3
  .brcase      2
  phvwr        p.tcp_option_nop_valid, 1
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
  // Update the bits for updating TCP Checksum 
  nop.c1.e
  nop
 
  // Now we need to update all the packet lengths, It could be add or subtract 
  sub          r3, r1, r2

  add          r4, k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, capri_p4_intrinsic_packet_len_sbit6_ebit13}, r3.s
  phvwr        p.capri_p4_intrinsic_packet_len, r4
  add          r4, k.ipv4_totalLen, r3.s
  phvwr        p.ipv4_totalLen, r4
  phvwr        p.tcp_dataOffset, r1[5:2]
  seq          c1, k.tunnel_metadata_tunnel_terminate, TRUE
  add.c1       r4, k.udp_len, r3.s
  phvwr.c1     p.udp_len, r4
  add.c1       r4, k.inner_ipv4_totalLen, r3.s
  phvwr.c1     p.inner_ipv4_totalLen, r4
  nop.e
  nop

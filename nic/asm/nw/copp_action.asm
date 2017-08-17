#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct copp_action_k k;
struct copp_action_d d;
struct phv_          p;

%%

copp_action:
  seq         c1, k.copp_metadata_policer_color, POLICER_COLOR_RED
  bcf         [c1], copp_deny
  add         r7, d.copp_action_d.permitted_packets, 1
  bgti        r7, 0xF, copp_permitted_stats_overflow
  tblwr       d.copp_action_d.permitted_packets, r6[3:0]
  tbladd.e    d.copp_action_d.permitted_bytes, k.control_metadata_packet_len
  nop

copp_permitted_stats_overflow:
  add         r7, d.copp_action_d.permitted_bytes, k.control_metadata_packet_len
  addi        r6, r0, 0x10000F
  or          r7, r7, r6, 32
  add         r5, r5, k.copp_metadata_policer_index, 5
  memwr.d.e   r5, r7
  tblwr       d.copp_action_d.permitted_bytes, 0

copp_deny:
  add         r7, d.copp_action_d.denied_packets, 1
  bgti        r7, 0xF, copp_denied_stats_overflow
  tblwr       d.copp_action_d.denied_packets, r6[3:0]
  tbladd.e    d.copp_action_d.denied_bytes, k.control_metadata_packet_len
  nop

copp_denied_stats_overflow:
  add         r7, d.copp_action_d.denied_bytes, k.control_metadata_packet_len
  addi        r6, r0, 0x10000F
  or          r7, r7, r6, 32
  add         r5, r5, k.copp_metadata_policer_index, 5
  add         r5, r5, 16
  memwr.d.e   r5, r7
  tblwr       d.copp_action_d.denied_bytes, 0

/*
 * stats allocation in the atomic add region:
 * 8B permit bytes, 8B permit packets, 8B deny bytes, 8B deby packets
 * total per policer index = 32B
 */

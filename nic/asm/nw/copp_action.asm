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
  memwr.d     r5[31:0], r7
  add         r6, r5[31:0], 8
  memwr.d.e   r6, 0xF
  tblwr       d.copp_action_d.permitted_bytes, 0

copp_deny:
  add         r7, d.copp_action_d.denied_packets, 1
  bgti        r7, 0xF, copp_denied_stats_overflow
  tblwr       d.copp_action_d.denied_packets, r6[3:0]
  tbladd.e    d.copp_action_d.denied_bytes, k.control_metadata_packet_len
  nop

copp_denied_stats_overflow:
  add         r7, d.copp_action_d.denied_bytes, k.control_metadata_packet_len
  memwr.d     r5[63:32], r7
  add         r6, r5[63:32], 8
  memwr.d.e   r6, 0xF
  tblwr       d.copp_action_d.denied_bytes, 0

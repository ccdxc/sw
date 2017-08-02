#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct egress_policer_action_k k;
struct egress_policer_action_d d;
struct phv_                    p;

%%

egress_policer_action:
  seq         c1, k.policer_metadata_egress_policer_color, POLICER_COLOR_GREEN
  bcf         [!c1], egress_policer_deny
  add         r7, d.egress_policer_action_d.permitted_packets, 1
  bgti        r7, 0xF, egress_policer_permitted_stats_overflow
  tblwr       d.egress_policer_action_d.permitted_packets, r6[3:0]
  tbladd.e    d.egress_policer_action_d.permitted_bytes, k.control_metadata_packet_len
  nop

egress_policer_permitted_stats_overflow:
  add         r7, d.egress_policer_action_d.permitted_bytes, k.control_metadata_packet_len
  memwr.d     r5[31:0], r7
  add         r6, r5[31:0], 8
  memwr.d.e   r6, 0xF
  tblwr       d.egress_policer_action_d.permitted_bytes, 0

egress_policer_deny:
  add         r7, d.egress_policer_action_d.denied_packets, 1
  bgti        r7, 0xF, egress_policer_denied_stats_overflow
  tblwr       d.egress_policer_action_d.denied_packets, r6[3:0]
  tbladd.e    d.egress_policer_action_d.denied_bytes, k.control_metadata_packet_len
  nop

egress_policer_denied_stats_overflow:
  add         r7, d.egress_policer_action_d.denied_bytes, k.control_metadata_packet_len
  memwr.d     r5[63:32], r7
  add         r6, r5[63:32], 8
  memwr.d.e   r6, 0xF
  tblwr       d.egress_policer_action_d.denied_bytes, 0

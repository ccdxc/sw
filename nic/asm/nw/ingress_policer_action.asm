#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct ingress_policer_action_k k;
struct ingress_policer_action_d d;
struct phv_                     p;

%%

ingress_policer_action:
  seq         c1, k.control_metadata_drop_reason[DROP_INGRESS_POLICER], 1
  bcf         [c1], ingress_policer_deny
  add         r7, d.ingress_policer_action_d.permitted_packets, 1
  phvwr       p.qos_metadata_cos_en, d.ingress_policer_action_d.cos_en
  phvwr       p.qos_metadata_cos, d.ingress_policer_action_d.cos
  phvwr       p.qos_metadata_dscp_en, d.ingress_policer_action_d.dscp_en
  phvwr       p.qos_metadata_dscp, d.ingress_policer_action_d.dscp
  bgti        r7, 0xF, ingress_policer_permitted_stats_overflow
  tblwr       d.ingress_policer_action_d.permitted_packets, r6[3:0]
  tbladd.e    d.ingress_policer_action_d.permitted_bytes, k.control_metadata_packet_len
  nop

ingress_policer_permitted_stats_overflow:
  add         r7, d.ingress_policer_action_d.permitted_bytes, k.control_metadata_packet_len
  addi        r6, r0, 0x10000F
  or          r7, r7, r6, 32
  add         r5, r5, k.policer_metadata_ingress_policer_index, 5
  memwr.d.e   r5, r7
  tblwr       d.ingress_policer_action_d.permitted_bytes, 0

ingress_policer_deny:
  add         r7, d.ingress_policer_action_d.denied_packets, 1
  bgti        r7, 0xF, ingress_policer_denied_stats_overflow
  tblwr       d.ingress_policer_action_d.denied_packets, r6[3:0]
  tbladd.e    d.ingress_policer_action_d.denied_bytes, k.control_metadata_packet_len
  nop

ingress_policer_denied_stats_overflow:
  add         r7, d.ingress_policer_action_d.denied_bytes, k.control_metadata_packet_len
  addi        r6, r0, 0x10000F
  or          r7, r7, r6, 32
  add         r5, r5, k.policer_metadata_ingress_policer_index, 5
  add         r5, r5, 16
  memwr.d.e   r5, r7
  tblwr       d.ingress_policer_action_d.denied_bytes, 0

/*
 * stats allocation in the atomic add region:
 * 8B permit bytes, 8B permit packets, 8B deny bytes, 8B deby packets
 * total per policer index = 32B
 */

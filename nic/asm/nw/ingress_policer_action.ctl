#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct ingress_policer_action_k k;
struct ingress_policer_action_d d;
struct phv_                     p;

k = {
  control_metadata_drop_reason[DROP_INGRESS_POLICER] = 1;
  control_metadata_packet_len = 0x80;
};

d = {
  ingress_policer_action_d.cos_en = 1;
  ingress_policer_action_d.cos = 0x3;
  ingress_policer_action_d.dscp_en = 1;
  ingress_policer_action_d.dscp = 0xa;
  ingress_policer_action_d.permitted_packets = 0xf;
  ingress_policer_action_d.permitted_bytes = 0x2345;
  ingress_policer_action_d.denied_packets = 0xf;
  ingress_policer_action_d.denied_bytes = 0x2345;
};

r5 = 0x12345600;

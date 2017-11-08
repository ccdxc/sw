#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct copp_action_k k;
struct copp_action_d d;
struct phv_          p;

k = {
  copp_metadata_policer_index = 50;
  copp_metadata_policer_color = POLICER_COLOR_GREEN;
  capri_p4_intrinsic_packet_len_sbit0_ebit5 = 0;
  capri_p4_intrinsic_packet_len_sbit6_ebit13 = 0x80;
};

d = {
  copp_action_d.permitted_packets = 0xf;
  copp_action_d.permitted_bytes = 0x2345;
  copp_action_d.denied_packets = 0xf;
  copp_action_d.denied_bytes = 0x2345;
};

r5 = 0x12345600;

#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct flow_stats_k k;
struct flow_stats_d d;
struct phv_         p;

k = {
  capri_intrinsic_drop = FALSE;
  flow_info_metadata_flow_index = 0x1;
  capri_p4_intrinsic_packet_len_sbit0_ebit5 = 1;
  capri_p4_intrinsic_packet_len_sbit6_ebit13 = 0x00;
  control_metadata_drop_reason[DROP_FLOW_HIT] = 1;
};

d = {
  flow_stats_d.permit_packets = 0xe;
  flow_stats_d.permit_bytes = 0x707;
};

r5 = 0xdeadbeef;

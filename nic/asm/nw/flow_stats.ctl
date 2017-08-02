#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct flow_stats_k k;
struct flow_stats_d d;
struct phv_         p;

k = {
  capri_intrinsic_drop = FALSE;
  flow_info_metadata_flow_index = 1;
  control_metadata_packet_len = 0x100;
  control_metadata_drop_reason[DROP_FLOW_HIT] = 1;
};

d = {
  flow_stats_d.permit_packets = 0xe;
  flow_stats_d.permit_bytes = 0x707;
};

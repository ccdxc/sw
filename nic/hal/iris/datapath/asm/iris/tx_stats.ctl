#include "egress.h"
#include "EGRESS_p.h"

struct tx_stats_k k;
struct tx_stats_d d;
struct phv_       p;

k = {
  capri_intrinsic_drop = 0;
  capri_p4_intrinsic_packet_len_sbit0_ebit5 = 0;
  capri_p4_intrinsic_packet_len_sbit6_ebit13 = 0x80;
  control_metadata_src_lif = 0xaa;
  flow_lkp_metadata_pkt_type = 0;
};

r5 = 0x5ead0000;

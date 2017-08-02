#include "egress.h"
#include "EGRESS_p.h"

struct tx_stats_k k;
struct tx_stats_d d;
struct phv_       p;

d = {
  tx_stats_d.tx_egress_drops = 0xffe;
  tx_stats_d.tx_ucast_pkts = 0x64;
  tx_stats_d.tx_mcast_pkts = 0x65;
  tx_stats_d.tx_bcast_pkts = 0x66;
  tx_stats_d.tx_ucast_bytes = 0xff0;
  tx_stats_d.tx_mcast_bytes = 0x3ff1;
  tx_stats_d.tx_bcast_bytes = 0x3ff2;
};

k = {
  capri_intrinsic_drop = 0;
  control_metadata_packet_len = 1500;
  control_metadata_src_lif = 0xaa;
  flow_lkp_metadata_pkt_type = 0;
};

r5 = 0xdead0000;

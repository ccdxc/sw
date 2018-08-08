#include "egress.h"
#include "EGRESS_p.h"

struct vnic_rx_stats_k   k;
struct vnic_rx_stats_d   d;
struct phv_              p;

d = {
    vnic_rx_stats_d.out_packets = 0x1;
    vnic_rx_stats_d.out_bytes = 0x200;
};

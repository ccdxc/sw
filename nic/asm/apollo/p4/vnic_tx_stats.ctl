#include "ingress.h"
#include "INGRESS_p.h"
#include "apollo.h"

struct vnic_tx_stats_k   k;
struct vnic_tx_stats_d   d;
struct phv_     p;

d = {
    vnic_tx_stats_d.in_packets = 0x20;
    vnic_tx_stats_d.in_bytes = 0x100;
};

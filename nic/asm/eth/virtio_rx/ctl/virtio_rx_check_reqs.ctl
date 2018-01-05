#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                 p;
struct rx_table_s2_t0_k     k;
struct rx_table_s2_t0_d     d;

#define D(field)    u.check_rx_reqs_d.##field

p = { 
    virtio_rx_global_phv_rx_virtq_used_pi = 0;
};

k = {
    to_s2_rx_virtq_avail_idx = 0x0005;
    to_s2_rx_virtq_avail_addr = 0x8000000012345678;
};


d = {
    D(rx_virtq_avail_ci) = 0x0400;
    D(rx_queue_size) = 0x0a00;
    D(rx_virtq_used_pi) = 0x0400;
};

params = {
    virtio_rx_read_head_desc_idx_start = 0x987654321;
};

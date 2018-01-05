#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                 p;
struct rx_table_s3_t0_k     k;
struct rx_table_s3_t0_d     d;

#define D(field)    u.read_rx_head_desc_idx_d.##field

p = { 
    virtio_rx_global_phv_head_desc_idx = 0;
};

k = {
    to_s3_rx_virtq_desc_addr = 0x8000000022345678; 
};


d = {
    D(desc_idx) = 0xff00;
};

params = {
    virtio_rx_read_head_desc_start = 0x987654321;
};

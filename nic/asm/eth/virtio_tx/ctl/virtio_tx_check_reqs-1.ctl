#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                 p;
struct tx_table_s2_t0_k     k;
struct tx_table_s2_t0_d     d;

#define D(field)    u.check_tx_reqs_d.##field

p = { 
    virtio_tx_global_phv_tx_virtq_used_pi = 0;
};

k = {
    to_s2_tx_virtq_avail_idx = 4;
    to_s2_lif_sbit0_ebit7 = 0x12;
    to_s2_lif_sbit8_ebit10 = 0x3;
    to_s2_qid = 0x01;
    to_s2_tx_virtq_avail_addr = 0x8000000000000000;
};

d = {
    D(tx_virtq_avail_ci) = 0x0400;
    D(tx_queue_size) = 0x0a00;
    D(tx_virtq_used_pi) = 0x0300;
};

params = {
    virtio_tx_read_head_desc_idx_start = 0x987654321;
};

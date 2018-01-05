#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                 p;
struct rx_table_s5_t0_k     k;
struct rx_table_s5_t0_d     d;

#define D(field)    u.read_rx_payload_desc_d.##field

p = { 
    p4_rxdma_intr_dma_cmd_ptr = 0;
};

k = {
    virtio_s2s_t0_phv_pkt_len = 0x0040;
    virtio_rx_global_phv_head_desc_idx = 0x00ff;
    to_s5_rx_virtq_used_addr = 0x8000000032345678;
    virtio_rx_global_phv_rx_virtq_used_pi = 0x0004;
    virtio_s2s_t0_phv_rx_queue_size = 0x000a;
    virtio_s2s_t0_phv_no_interrupt = 0;
};


d = {
    D(flags) = 0x0200;
    D(len) = 0x40000000;
    D(nextidx) = 0x0000;
    D(addr) = 0x8000000089abcdef;
};

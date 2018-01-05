#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                 p;
struct rx_table_s4_t0_k     k;
struct rx_table_s4_t0_d     d;

#define D(field)    u.read_rx_head_desc_d.##field

p = { 
    p4_rxdma_intr_dma_cmd_ptr = 0;
};

k = {
    to_s4_rx_virtq_desc_addr = 0x8000000022345678; 
    virtio_s2s_t0_phv_pkt_len = 0x0040;
    virtio_rx_global_phv_head_desc_idx = 0x00ff;
    to_s4_rx_virtq_used_addr = 0x8000000032345678;
    virtio_rx_global_phv_rx_virtq_used_pi = 0x0004;
    virtio_s2s_t0_phv_rx_queue_size = 0x000a;
    virtio_s2s_t0_phv_no_interrupt = 0;
};


d = {
    D(flags) = 0x0200;
    D(len) = 0x4a000000;
    D(nextidx) = 0x0000;
    D(addr) = 0x8000000089abcdef;
};

params = {
    virtio_rx_read_frag_desc_start = 0x987654321;
};

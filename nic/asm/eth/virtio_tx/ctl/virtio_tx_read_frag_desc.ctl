#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                 p;
struct tx_table_s5_t0_k     k;
struct tx_table_s5_t0_d     d;

#define D(field)    u.read_tx_payload_desc_d.##field

p = { 
    p4_txdma_intr_dma_cmd_ptr = 0;
};

k = {
    virtio_tx_global_phv_bytes_consumed = 0x000a;
    virtio_tx_global_phv_head_desc_idx = 0x00ff;
    to_s5_tx_virtq_used_addr = 0x8000000012345678;
    virtio_tx_global_phv_tx_virtq_used_pi = 0x0004;
    virtio_s2s_t0_phv_tx_queue_size = 0x000a;
    virtio_s2s_t0_phv_no_interrupt = 0;
};


d = {
    D(flags) = 0x0000;
    D(len) = 0x40000000;
    D(nextidx) = 0x0000;
    D(addr) = 0x8000000089abcdef;
};

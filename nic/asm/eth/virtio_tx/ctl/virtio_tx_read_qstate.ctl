#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                 p;
struct tx_table_s0_t0_k     k;
struct tx_table_s0_t0_d     d;

#define D(field)    u.read_virtq_tx_qstate_d.##field

r7 = 0x0000000000000001;

p = { 
    to_s2_qid = 0;
};

k = {
    p4_intr_global_lif_sbit0_ebit2 = 0x0;
    p4_intr_global_lif_sbit3_ebit10 = 0x12;
    p4_txdma_intr_qid = 0x0002;
    p4_txdma_intr_qstate_addr_sbit0_ebit1 = 0x00;
    p4_txdma_intr_qstate_addr_sbit2_ebit33 = 0x12345678;
};


d = {
    D(ci_0) = 0x0000;
    D(pi_0) = 0x0100;
    D(ci_1) = 0x0000;
    D(pi_1) = 0x0100;
    D(tx_virtq_avail_addr) = 0x8000000012345678;
    D(tx_virtq_desc_addr) = 0x8000000022345678;
    D(tx_virtq_used_addr) = 0x8000000032345678;
    D(tx_queue_size) = 0x0a00;
};

params = {
    virtio_tx_read_virtq_avail_start = 0x987654321;
};

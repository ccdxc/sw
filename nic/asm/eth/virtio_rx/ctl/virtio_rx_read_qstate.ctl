#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                 p;
struct common_p4plus_stage0_app_header_table_k  k;
struct common_p4plus_stage0_app_header_table_d  d;

#define D(field)    u.read_virtq_rx_qstate_d.##field

p = { 
    to_s2_qid = 0;
};

k = {
    p4_to_p4plus_packet_len = 0x0040;
    p4_rxdma_intr_qid = 0x0002;
    p4_intr_global_lif_sbit0_ebit2 = 0x00;
    p4_intr_global_lif_sbit3_ebit10 = 0x12;
    p4_rxdma_intr_qstate_addr_sbit0_ebit1 = 0x00;
    p4_rxdma_intr_qstate_addr_sbit2_ebit33 = 0x12345678;
};


d = {
    D(rx_msix_vector) = 0x0600;
    D(rx_virtq_avail_addr) = 0x8000000012345678;
    D(rx_virtq_desc_addr) = 0x8000000022345678;
    D(rx_virtq_used_addr) = 0x8000000032345678;
    D(rx_queue_size) = 0x0a00;
};

params = {
    virtio_rx_read_virtq_avail_start = 0x987654321;
};

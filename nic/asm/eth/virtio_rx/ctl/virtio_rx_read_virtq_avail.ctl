#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                 p;
struct rx_table_s1_t0_k     k;
struct rx_table_s1_t0_d     d;

#define D(field)    u.read_rx_virtq_avail_d.##field

p = { 
    to_s2_rx_virtq_avail_idx = 0;
};

k = {
    to_s1_qstate_addr_sbit0_ebit31= 0x12345678;
    to_s1_qstate_addr_sbit32_ebit35= 0x00;
};


d = {
    D(idx) = 0x0400;
    D(flags) = 0x0000;
};

params = {
    virtio_rx_check_reqs_start = 0x987654321;
};

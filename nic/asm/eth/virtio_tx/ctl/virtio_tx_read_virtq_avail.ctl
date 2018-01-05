#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                 p;
struct tx_table_s1_t0_k     k;
struct tx_table_s1_t0_d     d;

#define D(field)    u.read_tx_virtq_avail_d.##field

p = { 
    to_s2_tx_virtq_avail_idx = 0;
    virtio_s2s_t0_phv_no_interrupt = 0;
};

k = {
    to_s1_qstate_addr_sbit0_ebit31 = 0x8000d0d0;
    to_s1_qstate_addr_sbit32_ebit35 = 0x7;
};

d = {
    D(idx) = 4;
    D(flags) = 0;
};

params = {
    virtio_tx_check_reqs_start = 0x987654321;
};

#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct r2n_nvme_cmd_k k;
struct r2n_nvme_cmd_handle_d d;
struct phv_ p;

k = {
    r2n_rbuf_hdr_addr = 0xb000f000;
    rcq_ent_qp_index = 1;
    intrinsic_table_addr_raw = 0xb000f060; // 0 = phv[5=128]
};

d = {
    src_queue_id = 4;
    ssd_handle = 3;
    io_priority = 2;
    is_read = 1;
    cmd_handle = 0xe000e000;
};


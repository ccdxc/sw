#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct r2n_hq_entry_k k;
struct r2n_hq_entry_qpop_d d;
struct phv_ p;

k = {
    intrinsic_table_addr_raw = 0xe2002018; // 0 = phv[5=128]
};

d = {
    cmd_handle = 0xe0008000;
    local_addr = 0xe0009000;
    data_size = 8192;
};


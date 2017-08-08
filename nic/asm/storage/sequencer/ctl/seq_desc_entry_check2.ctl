#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct seq_desc_entry_k k;
struct seq_desc_entry_check_d d;
struct phv_ p;

k = {
    intrinsic_table_addr_raw = 0xC0004000; // 0 = phv[5=128]
    seq_tbl_entry_addr = 0xC0002000;
    seq_desc_entry_addr = 0xC0004000;
};

d = {
    is_valid = 0;
    is_last = 0;
    is_done = 0;
    is_error = 0;
    rsvd = 0;
    desc_size = 128;
    desc_base_addr = 0xD0001000;
    next_sq_ctx_addr = 0xE0001000;
};


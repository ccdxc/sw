#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct seq_sq_entry_k k;
struct seq_sq_entry_qpop_d d;
struct phv_ p;

k = {
    intrinsic_table_addr_raw = 0xb20020c0; // 0 = phv[5=128]
};

d = {
    pdma_src_addr = 0xF2001000;
    pdma_dst_addr = 0x00640000;
    pdma_size = 4096;
    tbl_entry_db = 0x80000000b8001000;
    tbl_entry_db_val = 0x24001000;
    rsvd = 0;
};

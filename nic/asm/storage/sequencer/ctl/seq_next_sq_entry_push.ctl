#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct seq_next_sq_ctx_k k;
struct seq_next_sq_ctx_qpush_d d;
struct phv_ p;

k = {
    intrinsic_table_addr_raw = 0xe0001000; // 0 = phv[5=128]
    seq_tbl_entry_addr = 0xC0004040; // 241 = phv[1=0]
    seq_tbl_entry_db = 0xC0002000; // 177 = phv[1=64]
    seq_is_last = 0; // 176 = phv[1=193]
    seq_desc_size = 128; // 160 = phv[1=196]
    seq_desc_base_addr = 0xD0001000; // 96 = phv[1=212]

};

d = {
    idx = 1;
    state = 0;
    c_ndx = 1;
    p_ndx = 2;
    p_ndx_db = 0xD0011000;
    c_ndx_db = 0xD0012000;
    base_addr = 0xD0013000;
    num_entries = 5; // power of 2
    paired_q_idx = 0;
    rsvd = 0;
};

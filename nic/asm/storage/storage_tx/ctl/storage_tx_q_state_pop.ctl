#include "ingress.h"
#include "INGRESS_p.h"


struct s0_tbl_k k;
struct s0_tbl_q_state_check_d d;
struct phv_ p;

k = {
    common_te0_phv_table_addr = 0x96001000; 
};


d = {
    pc_offset = 0x0000000A;
    rsvd = 0;
    cosA = 0;
    cosB = 0;
    cos_sel = 0;
    eval_last = 0;
    total_rings = 1;
    host_rings = 1;
    pid = 0;
    p_ndx = 1;
    c_ndx = 0;
    w_ndx = 0;
    num_entries = 6; // power of 2
    base_addr = 0x8000000010008000; // host address
    entry_size = 64;
    next_pc = 0x0000b000; // 28 bit capri address
    dst_qaddr = 0xA0008000; // 34 bit capri address
    dst_lif = 5;
    dst_qtype = 0;
    dst_qid = 0;
    vf_id = 0;
    sq_id = 0;
    pad = 0;
};

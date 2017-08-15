#include "ingress.h"
#include "INGRESS_p.h"


struct s2_tbl_k k;
struct s2_tbl_q_state_push_d d;
struct phv_ p;

k = {
    storage_kivec0_dst_qtype = 0;
    storage_kivec0_dst_lif_sbit8_ebit10 = 0x5;
    storage_kivec0_dst_qaddr_sbit0_ebit31 = 0xa0008000;
};


d = {
    pc_offset = 0x000000A0;
    rsvd = 0;
    cosA = 0;
    cosB = 0;
    cos_sel = 0;
    eval_last = 0;
    total_rings = 1;
    host_rings = 1;
    pid = 0;
    p_ndx = 2;
    c_ndx = 0;
    w_ndx = 0;
    num_entries = 6; // power of 2
    base_addr = 0x8002000020008000; // host address
    entry_size = 88;
    next_pc = 0x0000b000; // 28 bit capri address
    dst_qaddr = 0x0; // 34 bit capri address
    dst_lif = 0;
    dst_qtype = 0;
    dst_qid = 0;
    vf_id = 0;
    sq_id = 0;
    pad = 0;
};

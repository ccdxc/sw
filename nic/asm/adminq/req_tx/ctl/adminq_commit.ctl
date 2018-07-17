
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s3_t0_k.h"


struct phv_ p;
struct tx_table_s3_t0_k_ k;
struct tx_table_s3_t0_adminq_commit_d d;

k = {
    adminq_t0_s2s_lif = 0x1;
    adminq_t0_s2s_qtype = 0x2;
    adminq_t0_s2s_qid = 0x3;
    adminq_t0_s2s_adminq_qstate_addr = 0x10101010;
    adminq_t0_s2s_nicmgr_qstate_addr = 0xdeaddead;
    adminq_global_dma_cur_flit = 0x9;
    adminq_global_dma_cur_index = 0;
    adminq_to_s3_adminq_ci = 0x100;
};

d = {
    p_index0 = 0x200;
    c_index0 = 0x100;
    comp_index = 0;
    ci_fetch = 0x200;
    enable = 0x1;
    color = 0x1;
    ring_base = 0xbababa00;
    ring_size = 0xa00;
    nicmgr_qstate_addr = 0xdeaddead;
};


#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s1_t0_k.h"


struct phv_ p;
struct tx_table_s1_t0_k_ k;
struct tx_table_s1_t0_adminq_process_desc_d d;

k = {
    adminq_t0_s2s_lif = 0x1;
    adminq_t0_s2s_qtype = 0x2;
    adminq_t0_s2s_qid = 0x3;
    adminq_t0_s2s_adminq_qstate_addr = 0x10101010;
    adminq_t0_s2s_nicmgr_qstate_addr = 0xdeaddead;
};

d = {
    opcode = 0x80;
    data = 0xda;
};

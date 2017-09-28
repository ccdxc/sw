
#include "ingress.h"
#include "INGRESS_p.h"

struct phv_ p;
struct tx_table_s1_t0_k k;
struct tx_table_s1_t0_adminq_process_desc_d d;

k = {
    adminq_to_s1_cq_ring_base = 0xaa8000aa;
};

d = {
    opcode = 0x80;
    cmd_data0 = 0x90;
    cmd_data1 = 0xa0;
    cmd_data2 = 0xb0;
};

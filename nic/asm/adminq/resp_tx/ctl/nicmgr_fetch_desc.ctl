
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s0_t0_k.h"


struct phv_ p;
struct tx_table_s0_t0_k_ k;
struct tx_table_s0_t0_nicmgr_fetch_desc_d d;


k = {
	p4_intr_global_lif = 1;
	p4_txdma_intr_qtype = 2;
	p4_txdma_intr_qid = 3;
	p4_txdma_intr_qstate_addr = 0x10101010;
};

d = {
	p_index0 = 0x200;
	c_index0 = 0x100;
	ci_fetch = 0x100;
	comp_index = 0x0;
	enable = 1;
	ring_base = 0xbababa00;
	ring_size = 0x0a00;
};

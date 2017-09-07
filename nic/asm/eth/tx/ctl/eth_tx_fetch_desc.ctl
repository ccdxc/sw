
#include "ingress.h"
#include "INGRESS_p.h"

struct phv_ p;
struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_eth_tx_fetch_desc_d d;

p = {
	p4_txdma_intr_qtype = 2;
	p4_txdma_intr_qid = 0;
};

k = {
	p4_txdma_intr_qtype = 2;
	p4_txdma_intr_qid = 0;
};

d = {
	pc = 0xba;
	rsvd = 0xba;
	enable = 1;
	p_index0 = 1;
	c_index0 = 0;
	ring_base = 0x8000000000002000;
};

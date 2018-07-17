
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s0_t0_k.h"


struct phv_ p;
struct tx_table_s0_t0_k_ k;
struct tx_table_s0_t0_adminq_fetch_desc_d d;


k = {
	p4_intr_global_lif = 1;
	p4_txdma_intr_qtype = 2;
	p4_txdma_intr_qid = 3;
	p4_txdma_intr_qstate_addr = 0x301020304;
};

d = {
	p_index0 = 0x200;
	c_index0 = 0x100;
	ci_fetch = 0x100;
	comp_index = 0x0;
	enable = 0x1;
	color = 0x1;
	ring_base = 0x0102030405060708;
	ring_size = 0x0a00;
	intr_assert_addr = 0xdededede;
	nicmgr_qstate_addr = 0x0301020304000000;
};

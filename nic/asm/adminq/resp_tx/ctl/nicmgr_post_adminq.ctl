
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s2_t0_k.h"

struct phv_ p;
struct tx_table_s2_t0_k_ k;
struct tx_table_s2_t0_nicmgr_post_adminq_d d;

k = {
    nicmgr_global_dma_cur_flit = 0x7;
    nicmgr_global_dma_cur_index = 0;
    nicmgr_t0_s2s_lif = 0x1;
    nicmgr_t0_s2s_qtype = 0x2;
    nicmgr_t0_s2s_qid = 0x3;
};

d = {
	p_index0 = 0x200;
	c_index0 = 0x200;
	comp_index = 0x0100;
	enable = 1;
	color = 1;
	ring_base = 0xbababa00;
	ring_size = 0x0a00;
	intr_assert_addr = 0xdededede;
	nicmgr_qstate_addr = 0xdeaddead;
};

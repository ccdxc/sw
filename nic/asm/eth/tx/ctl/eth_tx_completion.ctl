
#include "INGRESS_p.h"
#include "INGRESS_tx_table_s7_t0_k.h"

#include "../../asm/eth/tx/defines.h"

struct phv_ p;
struct tx_table_s7_t0_k_ k;

p = {
    eth_tx_global_dma_cur_index = 0x20;
    eth_tx_t0_s2s_intr_assert_index = 0x1;
    eth_tx_t0_s2s_intr_assert_data = 0x010000;
};

k = {
    eth_tx_global_dma_cur_index = 0x20;
    eth_tx_t0_s2s_intr_assert_index = 0x0;
    eth_tx_t0_s2s_intr_assert_data = 0x010000;
};

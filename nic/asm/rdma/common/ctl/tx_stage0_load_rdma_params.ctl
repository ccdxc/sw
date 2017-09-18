#include "common_txdma_actions/asm_out/ingress.h"

struct tx_stage0_lif_params_table_d d;

d = {
    u.tx_stage0_lif_rdma_params_d.pt_base_addr_page_id = 0x12345;
    u.tx_stage0_lif_rdma_params_d.log_num_pt_entries = 10;
    u.tx_stage0_lif_rdma_params_d.cqcb_base_addr_page_id = 0x6789a;
    u.tx_stage0_lif_rdma_params_d.log_num_cq_entries = 10;
    //u.tx_stage0_lif_rdma_params_d.prefetch_pool_base_addr_page_id = 0xbcdef;
    //u.tx_stage0_lif_rdma_params_d.log_num_prefetch_pool_entries = 10;
};

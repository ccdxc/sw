#include "common_rxdma_actions/asm_out/ingress.h"

struct rx_stage0_load_rdma_params_d d;

d = {
    rx_stage0_load_rdma_params_d.pt_base_addr_page_id = 0x12345;
    rx_stage0_load_rdma_params_d.log_num_pt_entries = 10;
    rx_stage0_load_rdma_params_d.cqcb_base_addr_page_id = 0x6789a;
    rx_stage0_load_rdma_params_d.log_num_cq_entries = 10;
    //rx_stage0_load_rdma_params_d.prefetch_pool_base_addr_page_id = 0xbcdef;
    //rx_stage0_load_rdma_params_d.log_num_prefetch_pool_entries = 10;
};

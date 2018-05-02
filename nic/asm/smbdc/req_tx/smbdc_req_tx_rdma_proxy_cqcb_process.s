#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "common_phv.h"

struct req_tx_phv_t p;
struct sqcb3_t d;
struct smbdc_req_tx_s1_t0_k k;

#define IN_P t0_s2s_rdma_proxy_cqcb_info

#define K_RDMA_CQ_PROXY_C_INDEX CAPRI_KEY_RANGE(IN_P, rdma_cq_proxy_cindex_sbit0_ebit7, rdma_cq_proxy_cindex_sbit8_ebit15)
#define K_WQE_CONTEXT_ADDR      CAPRI_KEY_RANGE(IN_P, wqe_context_addr_sbit0_ebit7, wqe_context_addr_sbit24_ebit31)

#define TO_RDMA_CQE_P t0_s2s_rdma_cqe_info
#define TO_S4_P       to_s4_to_stage

%%
    .param    smbdc_req_tx_rdma_cqe_process

.align
smbdc_req_tx_rdma_proxy_cqcb_process:

    
    add        r1, r0, K_RDMA_CQ_PROXY_C_INDEX
    sll        r2, r1, d.rdma_cq_log_wqe_size
    add        r2, r2, d.rdma_cq_base_addr

    //pass rdma cq doorbell info using to-stage to writeback stage
    phvwr      CAPRI_PHV_RANGE(TO_S4_P, rdma_cq_lif, rdma_cq_ring_id), d.{rdma_cq_lif...rdma_cq_ring_id}

    CAPRI_RESET_TABLE_0_ARG()

    CAPRI_SET_FIELD2(TO_RDMA_CQE_P, wqe_context_addr, K_WQE_CONTEXT_ADDR)

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, smbdc_req_tx_rdma_cqe_process, r2)

    nop.e
    nop

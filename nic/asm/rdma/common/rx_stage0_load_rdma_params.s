#include "capri.h"
#include "common_phv.h"
#include "common_rxdma_actions/asm_out/INGRESS_p.h"
#include "common_rxdma_actions/asm_out/ingress.h"

struct rx_stage0_load_rdma_params_k k;
struct rx_stage0_load_rdma_params_d d;
struct phv_ p;

%%

rx_stage0_load_rdma_params:

    add r1, r0, k.p4_intr_recirc
    bne r1, r0, done
    add r4, r0, k.p4_rxdma_intr_qtype //BD slot
    sllv r5, 1, r4
    and r5, r5, d.rx_stage0_load_rdma_params_d.rdma_en_qtype_mask
    seq c1, r5, r0
    bcf [c1], done

    add r1, r0, offsetof(struct phv_, common_global_global_data) //BD slot
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, pt_base_addr_page_id, d.rx_stage0_load_rdma_params_d.pt_base_addr_page_id)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, log_num_pt_entries, d.rx_stage0_load_rdma_params_d.log_num_pt_entries)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, cqcb_base_addr_page_id, d.rx_stage0_load_rdma_params_d.cqcb_base_addr_page_id)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, log_num_cq_entries, d.rx_stage0_load_rdma_params_d.log_num_cq_entries)
    #CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, prefetch_pool_base_addr_page_id, d.rx_stage0_load_rdma_params_d.prefetch_pool_base_addr_page_id)
    #CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, log_num_prefetch_pool_entries, d.rx_stage0_load_rdma_params_d.log_num_prefetch_pool_entries)
done:
    nop.e
    nop

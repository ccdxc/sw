#include "capri.h"
#include "common_phv.h"
#include "INGRESS_p.h"
#include "common.h"

struct tx_stage0_load_rdma_params_k_t k;
struct sram_lif_entry_t d;
struct phv_ p;

%%

tx_stage0_load_rdma_params:

    add r4, r0, k.global.qtype
    sllv r5, 1, r4
    and r5, r5, d.rdma_en_qtype_mask
    seq c1, r5, r0
    bcf [c1], done

    add r1, r0, offsetof(struct phv_, common_global_global_data)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, pt_base_addr_page_id, d.pt_base_addr_page_id)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, log_num_pt_entries, d.log_num_pt_entries)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, cqcb_base_addr_page_id, d.cqcb_base_addr_page_id)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, log_num_cq_entries, d.log_num_cq_entries)
    #CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, prefetch_pool_base_addr_page_id, d.prefetch_pool_base_addr_page_id)
    #CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, log_num_prefetch_pool_entries, d.log_num_prefetch_pool_entries)
done:
    nop.e
    nop

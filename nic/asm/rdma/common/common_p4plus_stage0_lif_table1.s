#include "capri.h"
#include "common_phv.h"
#include "INGRESS_p.h"

struct sram_lif_entry_t d;
struct phv_ p;

%%

common_p4plus_stage0_lif_table1:
    add r1, r0, offsetof(struct phv_, common_global_global_data)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, pt_base_addr_page_id, d.pt_base_addr_page_id)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, log_num_pt_entries, d.log_num_pt_entries)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, cqcb_base_addr_page_id, d.cqcb_base_addr_page_id)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, log_num_cq_entries, d.log_num_cq_entries)
    #CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, prefetch_pool_base_addr_page_id, d.prefetch_pool_base_addr_page_id)
    #CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, log_num_prefetch_pool_entries, d.log_num_prefetch_pool_entries)
    nop.e
    nop

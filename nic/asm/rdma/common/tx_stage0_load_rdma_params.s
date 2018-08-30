#include "capri.h"
#include "common_phv.h"
#include "common_txdma_actions/asm_out/INGRESS_p.h"
#include "common_txdma_actions/asm_out/ingress.h"
#include "req_tx_args.h"
#include "aq_tx_args.h"
    
struct tx_stage0_lif_params_table_k k;
struct tx_stage0_lif_params_table_d d;
struct phv_ p;

#define REQ_TX_TO_S2_T struct req_tx_to_stage_2_t
#define AQ_TX_TO_S1_T struct aq_tx_to_stage_wqe_info_t
#define AQ_TX_TO_S2_T struct aq_tx_to_stage_wqe2_info_t
    
%%

tx_stage0_load_rdma_params:

    add r4, r0, k.p4_txdma_intr_qtype //BD slot
    sllv r5, 1, r4
    and r5, r5, d.u.tx_stage0_lif_rdma_params_d.rdma_en_qtype_mask
    seq c1, r5, r0
    bcf [c1], done

    # is it adminq ?
    seq c2, k.p4_txdma_intr_qtype, d.u.tx_stage0_lif_rdma_params_d.aq_qtype // BD slot

    add r1, r0, offsetof(struct phv_, common_global_global_data)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, pt_base_addr_page_id, d.u.tx_stage0_lif_rdma_params_d.pt_base_addr_page_id)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, log_num_pt_entries, d.u.tx_stage0_lif_rdma_params_d.log_num_pt_entries)

    bcf [c2], aq

sq:
    add r1, r0, offsetof(struct phv_, to_stage_2_to_stage_data)
    CAPRI_SET_FIELD(r1, REQ_TX_TO_S2_T, ah_base_addr_page_id, d.u.tx_stage0_lif_rdma_params_d.ah_base_addr_page_id)
    b done
    nop // BD slot

aq:
    add r2, r0, offsetof(struct phv_, to_stage_1_to_stage_data)    
    CAPRI_SET_FIELD(r2, AQ_TX_TO_S1_T, cqcb_base_addr_hi, d.u.tx_stage0_lif_rdma_params_d.cqcb_base_addr_hi)
    CAPRI_SET_FIELD(r2, AQ_TX_TO_S1_T, sqcb_base_addr_hi, d.u.tx_stage0_lif_rdma_params_d.sqcb_base_addr_hi)
    CAPRI_SET_FIELD(r2, AQ_TX_TO_S1_T, rqcb_base_addr_hi, d.u.tx_stage0_lif_rdma_params_d.rqcb_base_addr_hi)

    add         r2, r0, offsetof(struct phv_, to_stage_2_to_stage_data)
    CAPRI_SET_FIELD(r2, AQ_TX_TO_S2_T, ah_base_addr_page_id, d.u.tx_stage0_lif_rdma_params_d.ah_base_addr_page_id)
    CAPRI_SET_FIELD(r2, AQ_TX_TO_S2_T, rrq_base_addr_page_id, d.u.tx_stage0_lif_rdma_params_d.rrq_base_addr_page_id)
    CAPRI_SET_FIELD(r2, AQ_TX_TO_S2_T, rsq_base_addr_page_id, d.u.tx_stage0_lif_rdma_params_d.rsq_base_addr_page_id)

done:
    nop.e
    nop

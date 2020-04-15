#include "capri.h"
#include "common_phv.h"
#include "common_txdma_actions/asm_out/INGRESS_p.h"
#include "common_txdma_actions/asm_out/ingress.h"
#include "req_tx_args.h"
#include "resp_tx_args.h"
#include "aq_tx_args.h"
#include "resp_tx_args.h"

struct tx_stage0_lif_params_table_k k;
struct tx_stage0_lif_params_table_d d;
struct phv_ p;

#define	RQCB_TO_DCQCN_CFG_T struct resp_tx_rqcb_to_dcqcn_config_t
#define REQ_TX_TO_S2_T struct req_tx_to_stage_2_t
#define AQ_TX_TO_S1_T struct aq_tx_to_stage_wqe_info_t
#define AQ_TX_TO_S2_T struct aq_tx_to_stage_wqe2_info_t
#define AQ_TX_TO_S3_T struct aq_tx_to_stage_sqcb_info_t
#define AQ_TX_S2S_T struct aq_tx_aqcb_to_wqe_t
#define RESP_TX_T2_S2S_T struct resp_tx_rqcb_to_precheckout_info_t

%%

tx_stage0_load_rdma_params:

    add r4, r0, k.p4_txdma_intr_qtype
    sllv r5, 1, r4
    and r5, r5, d.u.tx_stage0_lif_rdma_params_d.rdma_en_qtype_mask
    seq c1, r5, r0
    bcf [c1], done

    # is it adminq ?
    seq c2, k.p4_txdma_intr_qtype, d.u.tx_stage0_lif_rdma_params_d.aq_qtype // BD slot
    bcf [c2], aq
    add r1, r0, offsetof(struct phv_, common_global_global_data)    // BD slot

    seq c2, k.p4_txdma_intr_qtype, d.u.tx_stage0_lif_rdma_params_d.sq_qtype
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, pt_base_addr_page_id, d.u.tx_stage0_lif_rdma_params_d.pt_base_addr_page_id)
    bcf [c2], sq
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, log_num_pt_entries, d.u.tx_stage0_lif_rdma_params_d.log_num_pt_entries)    // BD Slot

rq:
    // prefetch related work
    add r2, r0, offsetof(struct phv_, common_t2_s2s_s2s_data)
    sll r1, d.u.tx_stage0_lif_rdma_params_d.prefetch_base_addr_page_id, (HBM_PAGE_SIZE_SHIFT - PT_BASE_ADDR_SHIFT)
    CAPRI_SET_FIELD(r2, RESP_TX_T2_S2S_T, prefetch_cb_or_base_addr, r1)

    // r3 = PT table size
    add r3, CAPRI_LOG_SIZEOF_U64, d.u.tx_stage0_lif_rdma_params_d.log_num_pt_entries
    sllv r3, 1, r3
    // r4 = Key table base address
    add r4, r3, d.u.tx_stage0_lif_rdma_params_d.pt_base_addr_page_id, HBM_PAGE_SIZE_SHIFT
    sllv r3, 1, d.u.tx_stage0_lif_rdma_params_d.log_num_kt_entries
    // r4 = dcqcn_config table base address
    add r4, r4, r3, LOG_SIZEOF_KEY_ENTRY_T

    add r2, r0, offsetof(struct phv_, common_t1_s2s_s2s_data)
    b   done
    CAPRI_SET_FIELD(r2, RQCB_TO_DCQCN_CFG_T, dcqcn_config_base, r4)  // BD Slot

sq:
    add r2, r0, offsetof(struct phv_, to_stage_2_to_stage_data) // BD slot
    CAPRI_SET_FIELD(r2, REQ_TX_TO_S2_T, log_num_kt_entries, d.u.tx_stage0_lif_rdma_params_d.log_num_kt_entries)
    b done
    CAPRI_SET_FIELD(r2, REQ_TX_TO_S2_T, log_num_dcqcn_profiles, d.u.tx_stage0_lif_rdma_params_d.log_num_dcqcn_profiles)     // BD Slot

aq:
    add r2, r0, k.p4_txdma_intr_qid
    add r3, r0, RDMA_AQ_QID_START
    // Ethernet uses admin_qid 0. Skip the following for the same
    blt r2, r3, done

    add r2, r0, offsetof(struct phv_, to_stage_1_to_stage_data) // BD slot

    // All PHV writes should happen only after we ensure we are handling a RDMA AdminQ only
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, pt_base_addr_page_id, d.u.tx_stage0_lif_rdma_params_d.pt_base_addr_page_id)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, log_num_pt_entries, d.u.tx_stage0_lif_rdma_params_d.log_num_pt_entries)

    CAPRI_SET_FIELD(r2, AQ_TX_TO_S1_T, cqcb_base_addr_hi, d.u.tx_stage0_lif_rdma_params_d.cqcb_base_addr_hi)
    CAPRI_SET_FIELD(r2, AQ_TX_TO_S1_T, sqcb_base_addr_hi, d.u.tx_stage0_lif_rdma_params_d.sqcb_base_addr_hi)
    CAPRI_SET_FIELD(r2, AQ_TX_TO_S1_T, rqcb_base_addr_hi, d.u.tx_stage0_lif_rdma_params_d.rqcb_base_addr_hi)
    CAPRI_SET_FIELD(r2, AQ_TX_TO_S1_T, log_num_cq_entries, d.u.tx_stage0_lif_rdma_params_d.log_num_cq_entries)
    CAPRI_SET_FIELD(r2, AQ_TX_TO_S1_T, log_num_kt_entries, d.u.tx_stage0_lif_rdma_params_d.log_num_kt_entries)
    CAPRI_SET_FIELD(r2, AQ_TX_TO_S1_T, log_num_dcqcn_profiles, d.u.tx_stage0_lif_rdma_params_d.log_num_dcqcn_profiles)
    CAPRI_SET_FIELD(r2, AQ_TX_TO_S1_T, ah_base_addr_page_id, d.u.tx_stage0_lif_rdma_params_d.ah_base_addr_page_id)
    CAPRI_SET_FIELD_RANGE(r2, AQ_TX_TO_S1_T, barmap_base, barmap_size, d.{u.tx_stage0_lif_rdma_params_d.barmap_base_addr...u.tx_stage0_lif_rdma_params_d.barmap_size})

    add         r2, r0, offsetof(struct phv_, common_t0_s2s_s2s_data)
    CAPRI_SET_FIELD(r2, AQ_TX_S2S_T, log_num_ah_entries, d.u.tx_stage0_lif_rdma_params_d.log_num_ah_entries)
    CAPRI_SET_FIELD(r2, AQ_TX_S2S_T, log_num_eq_entries, d.u.tx_stage0_lif_rdma_params_d.log_num_eq_entries)
    CAPRI_SET_FIELD(r2, AQ_TX_S2S_T, log_num_sq_entries, d.u.tx_stage0_lif_rdma_params_d.log_num_sq_entries)
    CAPRI_SET_FIELD(r2, AQ_TX_S2S_T, log_num_rq_entries, d.u.tx_stage0_lif_rdma_params_d.log_num_rq_entries)

    add         r2, r0, offsetof(struct phv_, to_stage_2_to_stage_data)
    CAPRI_SET_FIELD(r2, AQ_TX_TO_S2_T, ah_base_addr_page_id, d.u.tx_stage0_lif_rdma_params_d.ah_base_addr_page_id)
    CAPRI_SET_FIELD(r2, AQ_TX_TO_S2_T, sqcb_base_addr_hi, d.u.tx_stage0_lif_rdma_params_d.sqcb_base_addr_hi)
    CAPRI_SET_FIELD(r2, AQ_TX_TO_S2_T, rqcb_base_addr_hi, d.u.tx_stage0_lif_rdma_params_d.rqcb_base_addr_hi)
    CAPRI_SET_FIELD(r2, AQ_TX_TO_S2_T, log_num_kt_entries, d.u.tx_stage0_lif_rdma_params_d.log_num_kt_entries)
    CAPRI_SET_FIELD(r2, AQ_TX_TO_S2_T, log_num_dcqcn_profiles, d.u.tx_stage0_lif_rdma_params_d.log_num_dcqcn_profiles)

    add         r2, r0, offsetof(struct phv_, to_stage_3_to_stage_data)
    CAPRI_SET_FIELD(r2, AQ_TX_TO_S3_T, rqcb_base_addr_hi, d.u.tx_stage0_lif_rdma_params_d.rqcb_base_addr_hi)

done:
    nop.e
    nop

#include "capri.h"
#include "common_phv.h"
#include "common_rxdma_actions/asm_out/INGRESS_p.h"
#include "common_rxdma_actions/asm_out/ingress.h"
#include "req_rx_args.h"
#include "resp_rx_args.h"

struct rx_stage0_load_rdma_params_k k;
struct rx_stage0_load_rdma_params_d d;
struct phv_ p;

#define REQ_RX_TO_STAGE_T struct req_rx_to_stage_t
#define RESP_RX_TO_S3_T struct resp_rx_s3_info_t
#define RESP_RX_TO_S5_T struct resp_rx_s5_info_t

%%

rx_stage0_load_rdma_params:

    add r1, r0, k.p4_intr_recirc
    bne r1, r0, done
    add r4, r0, k.p4_rxdma_intr_qtype //BD slot
    sllv r5, 1, r4
    and r5, r5, d.u.rx_stage0_load_rdma_params_d.rdma_en_qtype_mask
    seq c1, r5, r0
    bcf [c1], done

    # is it sq or rq ?
    seq c2, k.p4_rxdma_intr_qtype, d.u.rx_stage0_load_rdma_params_d.sq_qtype

    add r1, r0, offsetof(struct phv_, common_global_global_data)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, pt_base_addr_page_id, d.u.rx_stage0_load_rdma_params_d.pt_base_addr_page_id)
    bcf [c2], sq
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, log_num_pt_entries, d.u.rx_stage0_load_rdma_params_d.log_num_pt_entries)   //BD Slot

rq:
    #add r1, r0, offsetof(struct phv_, to_stage_2_to_stage_data)
    #CAPRI_SET_FIELD(r1, TO_S_EXT_HDR_INFO_T, ext_hdr_data, k.{ext_app_header_app_data3_sbit0_ebit31...ext_app_header_app_data3_sbit32_ebit63})
    phvwr p.to_stage_2_to_stage_data, k.ext_app_header_app_data3

    // copy to stage 3 and 5
    add r2, r0, offsetof(struct phv_, to_stage_3_to_stage_data)
    CAPRI_SET_FIELD(r2, RESP_RX_TO_S3_T, wb1.cqcb_base_addr_page_id, d.u.rx_stage0_load_rdma_params_d.cqcb_base_addr_page_id)
    CAPRI_SET_FIELD(r2, RESP_RX_TO_S3_T, wb1.log_num_cq_entries, d.u.rx_stage0_load_rdma_params_d.log_num_cq_entries)

    add r2, r0, offsetof(struct phv_, to_stage_5_to_stage_data)
    CAPRI_SET_FIELD(r2, RESP_RX_TO_S5_T, cqpt.cqcb_base_addr_page_id, d.u.rx_stage0_load_rdma_params_d.cqcb_base_addr_page_id)
    CAPRI_SET_FIELD(r2, RESP_RX_TO_S5_T, cqpt.log_num_cq_entries, d.u.rx_stage0_load_rdma_params_d.log_num_cq_entries)
    b   done
    nop

sq:
    // copy to stage 1,2,3,4
    add r2, r0, offsetof(struct phv_, to_stage_1_to_stage_data)
    CAPRI_SET_FIELD(r2, REQ_RX_TO_STAGE_T, cqcb_base_addr_page_id, d.u.rx_stage0_load_rdma_params_d.cqcb_base_addr_page_id)
    CAPRI_SET_FIELD(r2, REQ_RX_TO_STAGE_T, log_num_cq_entries, d.u.rx_stage0_load_rdma_params_d.log_num_cq_entries)

    add r2, r0, offsetof(struct phv_, to_stage_2_to_stage_data)
    CAPRI_SET_FIELD(r2, REQ_RX_TO_STAGE_T, cqcb_base_addr_page_id, d.u.rx_stage0_load_rdma_params_d.cqcb_base_addr_page_id)
    CAPRI_SET_FIELD(r2, REQ_RX_TO_STAGE_T, log_num_cq_entries, d.u.rx_stage0_load_rdma_params_d.log_num_cq_entries)

    add r2, r0, offsetof(struct phv_, to_stage_3_to_stage_data)
    CAPRI_SET_FIELD(r2, REQ_RX_TO_STAGE_T, cqcb_base_addr_page_id, d.u.rx_stage0_load_rdma_params_d.cqcb_base_addr_page_id)
    CAPRI_SET_FIELD(r2, REQ_RX_TO_STAGE_T, log_num_cq_entries, d.u.rx_stage0_load_rdma_params_d.log_num_cq_entries)

    add r2, r0, offsetof(struct phv_, to_stage_4_to_stage_data)
    CAPRI_SET_FIELD(r2, REQ_RX_TO_STAGE_T, cqcb_base_addr_page_id, d.u.rx_stage0_load_rdma_params_d.cqcb_base_addr_page_id)
    CAPRI_SET_FIELD(r2, REQ_RX_TO_STAGE_T, log_num_cq_entries, d.u.rx_stage0_load_rdma_params_d.log_num_cq_entries)

    #CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, prefetch_pool_base_addr_page_id, d.u.rx_stage0_load_rdma_params_d.prefetch_pool_base_addr_page_id)
    #CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, log_num_prefetch_pool_entries, d.u.rx_stage0_load_rdma_params_d.log_num_prefetch_pool_entries)
done:
    nop.e
    nop

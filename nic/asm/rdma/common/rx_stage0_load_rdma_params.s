#include "capri.h"
#include "common_phv.h"
#include "common_rxdma_actions/asm_out/INGRESS_p.h"
#include "common_rxdma_actions/asm_out/ingress.h"
#include "req_rx_args.h"
#include "resp_rx_args.h"

#include "aq_rx_args.h"    

struct      rx_stage0_load_rdma_params_k k ;
struct      rx_stage0_load_rdma_params_d d ;
struct phv_ p;

#define REQ_RX_TO_S5_T struct req_rx_to_stage_cq_info_t
#define REQ_RX_TO_S6_T struct req_rx_to_stage_cq_info_t
#define RESP_RX_TO_S5_T struct resp_rx_to_stage_wb1_info_t
#define RESP_RX_TO_S6_T struct resp_rx_to_stage_cqpt_info_t

#define AQ_RX_TO_S1_T struct aq_rx_to_stage_wqe_t
#define AQ_RX_TO_S3_T struct aq_rx_to_stage_sqcb1_t
#define AQ_RX_TO_S4_T struct aq_rx_to_stage_sqcb1_t
#define AQ_RX_TO_S5_T struct aq_rx_to_stage_cqcb_t    
#define AQ_RX_TO_S6_T struct aq_rx_to_stage_cqcb_t
    
%%

rx_stage0_load_rdma_params:

    add r1, r0, k.p4_intr_recirc
    bne r1, r0, done
    add r4, r0, k.p4_rxdma_intr_qtype //BD slot
    sllv r5, 1, r4
    and r5, r5, d.u.rx_stage0_load_rdma_params_d.rdma_en_qtype_mask
    seq c1, r5, r0
    bcf [c1], done

    # is it aq ?
    seq c3, k.p4_rxdma_intr_qtype, d.u.rx_stage0_load_rdma_params_d.aq_qtype
    bcf         [c3], aq

    add r1, r0, offsetof(struct phv_, common_global_global_data)    // BD slot
    # is it sq or rq ?
    seq c2, k.p4_rxdma_intr_qtype, d.u.rx_stage0_load_rdma_params_d.sq_qtype

    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, pt_base_addr_page_id, d.u.rx_stage0_load_rdma_params_d.pt_base_addr_page_id)
    bcf [c2], sq
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, log_num_pt_entries, d.u.rx_stage0_load_rdma_params_d.log_num_pt_entries)   //BD Slot

rq:
    #add r1, r0, offsetof(struct phv_, to_stage_2_to_stage_data)
    #CAPRI_SET_FIELD(r1, TO_S_EXT_HDR_INFO_T, ext_hdr_data, k.{ext_app_header_app_data3_sbit0_ebit31...ext_app_header_app_data3_sbit32_ebit63})
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, pad.resp_rx.log_num_kt_entries, d.u.rx_stage0_load_rdma_params_d.log_num_kt_entries)

    // stage 2 for rqwqe
    // Accomodate smac (32 bits) + vlan_ethertype (16 bits) + vlan (16 bits) + ethertype (5 bits) for UD packets
    phvwr p.to_stage_2_to_stage_data[127:59], k.ext_app_header_app_data3[95:27]
    // stage 2 for rqrkey
    //phvwr p.to_stage_2_to_stage_data[127:64], k.ext_app_header_app_data3[95:32]

    // copy to stage 1, 5 and 6
    add r2, r0, offsetof(struct phv_, to_stage_1_to_stage_data)

    add r2, r0, offsetof(struct phv_, to_stage_5_to_stage_data)
    CAPRI_SET_FIELD(r2, RESP_RX_TO_S5_T, cqcb_base_addr_hi, d.u.rx_stage0_load_rdma_params_d.cqcb_base_addr_hi)
    CAPRI_SET_FIELD(r2, RESP_RX_TO_S5_T, log_num_cq_entries, d.u.rx_stage0_load_rdma_params_d.log_num_cq_entries)    

    add r2, r0, offsetof(struct phv_, to_stage_6_to_stage_data)
    CAPRI_SET_FIELD(r2, RESP_RX_TO_S6_T, cqcb_base_addr_hi, d.u.rx_stage0_load_rdma_params_d.cqcb_base_addr_hi)
    CAPRI_SET_FIELD(r2, RESP_RX_TO_S6_T, log_num_cq_entries, d.u.rx_stage0_load_rdma_params_d.log_num_cq_entries)
    b   done
    nop

aq:
    add r2, r0, k.p4_rxdma_intr_qid
    add r3, r0, RDMA_AQ_QID_START
    # Ethernet uses admin_qid 0. Skip the following for the same
    blt r2, r3, done

    add r2, r0, offsetof(struct phv_, to_stage_5_to_stage_data)     // BD slot
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, pt_base_addr_page_id, d.u.rx_stage0_load_rdma_params_d.pt_base_addr_page_id)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, log_num_pt_entries, d.u.rx_stage0_load_rdma_params_d.log_num_pt_entries)
    // stage 3 for rqwqe
    phvwr p.to_stage_3_to_stage_data[127:96], k.ext_app_header_app_data3[31:0]
    // stage 2 for rqrkey
    phvwr p.to_stage_2_to_stage_data[127:64], k.ext_app_header_app_data3[95:32]

    // copy to stage 1
    add r2, r0, offsetof(struct phv_, to_stage_1_to_stage_data)    
    CAPRI_SET_FIELD(r2, AQ_RX_TO_S1_T, rqcb_base_addr_hi, d.u.rx_stage0_load_rdma_params_d.rqcb_base_addr_hi)
    CAPRI_SET_FIELD(r2, AQ_RX_TO_S1_T, prefetch_base_addr_page_id, d.u.rx_stage0_load_rdma_params_d.prefetch_base_addr_page_id)
    CAPRI_SET_FIELD(r2, AQ_RX_TO_S1_T, log_prefetch_buf_size, d.u.rx_stage0_load_rdma_params_d.log_prefetch_buf_size)
    // copy to stage 3
    add r2, r0, offsetof(struct phv_, to_stage_3_to_stage_data)    
    CAPRI_SET_FIELD(r2, AQ_RX_TO_S3_T, sqcb_base_addr_hi, d.u.rx_stage0_load_rdma_params_d.sqcb_base_addr_hi)
    // copy to stage 4
    add r2, r0, offsetof(struct phv_, to_stage_4_to_stage_data)    
    CAPRI_SET_FIELD(r2, AQ_RX_TO_S4_T, sqcb_base_addr_hi, d.u.rx_stage0_load_rdma_params_d.sqcb_base_addr_hi)
    CAPRI_SET_FIELD(r2, AQ_RX_TO_S4_T, rqcb_base_addr_hi, d.u.rx_stage0_load_rdma_params_d.rqcb_base_addr_hi)
    // copy to stage 6
    add r2, r0, offsetof(struct phv_, to_stage_6_to_stage_data)
    CAPRI_SET_FIELD(r2, AQ_RX_TO_S6_T, cqcb_base_addr_hi, d.u.rx_stage0_load_rdma_params_d.cqcb_base_addr_hi)
    CAPRI_SET_FIELD(r2, AQ_RX_TO_S6_T, log_num_cq_entries, d.u.rx_stage0_load_rdma_params_d.log_num_cq_entries)

    b           done
    nop
    
sq:
    // copy to stage 1,2,3,4, 5, 6
    add r2, r0, offsetof(struct phv_, to_stage_5_to_stage_data)
    CAPRI_SET_FIELD(r2, REQ_RX_TO_S5_T, cqcb_base_addr_hi, d.u.rx_stage0_load_rdma_params_d.cqcb_base_addr_hi)
    CAPRI_SET_FIELD(r2, REQ_RX_TO_S5_T, log_num_cq_entries, d.u.rx_stage0_load_rdma_params_d.log_num_cq_entries)
    
    add r2, r0, offsetof(struct phv_, to_stage_6_to_stage_data)
    CAPRI_SET_FIELD(r2, REQ_RX_TO_S6_T, cqcb_base_addr_hi, d.u.rx_stage0_load_rdma_params_d.cqcb_base_addr_hi)
    CAPRI_SET_FIELD(r2, REQ_RX_TO_S6_T, log_num_cq_entries, d.u.rx_stage0_load_rdma_params_d.log_num_cq_entries)

done:
    nop.e
    nop

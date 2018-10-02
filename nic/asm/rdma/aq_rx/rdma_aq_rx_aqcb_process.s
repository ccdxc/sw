#include "aq_rx.h"
#include "aqcb.h"

struct aq_rx_phv_t p;
struct aqcb_t d;
struct common_p4plus_stage0_app_header_table_k k;

#define PHV_GLOBAL_COMMON_P phv_global_common

#define AQCB_TO_CQ_P t2_s2s_aqcb_to_cq_info
#define AQCB_TO_WQE_P t3_s2s_aqcb_to_wqe_info

#define TO_S6_INFO to_s6_info
    
#define K_OP k.{rdma_aq_feedback_op_sbit0_ebit6,rdma_aq_feedback_op_sbit7_ebit7}
#define K_PD k.{rdma_aq_feedback_qp_pd_sbit0_ebit15,rdma_aq_feedback_qp_pd_sbit16_ebit31}
#define K_RQ_MAP_COUNT k.rdma_aq_feedback_qp_rq_map_count
#define K_RQ_DMA_ADDR k.{rdma_aq_feedback_qp_rq_dma_addr_sbit0_ebit15...rdma_aq_feedback_qp_rq_dma_addr_sbit48_ebit63}
#define K_RQ_CQ_ID k.{rdma_aq_feedback_qp_rq_cq_id_sbit0_ebit7...rdma_aq_feedback_qp_rq_cq_id_sbit16_ebit23}

#define K_RQ_ID k.{rdma_aq_feedback_qp_rq_id_sbit0_ebit15,rdma_aq_feedback_qp_rq_id_sbit16_ebit23}
    
%%

    .param      rdma_aq_rx_cqcb_process
    .param      rdma_aq_rx_wqe_process
    .param      rdma_resp_rx_stage0
    .param      rx_dummy
.align
rdma_aq_rx_aqcb_process:

    add            r1, r0, CAPRI_APP_DATA_RAW_FLAGS
    bnei           r1, AQ_RX_FLAG_RDMA_FEEDBACK, exit
    CAPRI_SET_TABLE_0_VALID(0) //BD Slot

process_feedback:
    
    seq         c1, k.rdma_aq_feedback_feedback_type, RDMA_AQ_FEEDBACK
    bcf         [!c1], exit
    
    seq         c1, K_OP, AQ_OP_TYPE_CREATE_QP
    bcf         [c1], create_qp
    nop
    
aq_feedback:
    
    #CQCB is loaded by req_rx and resp_rx flows in stage-5, table-2 for mutual
    #exclusive access. Do the same for aq_rx also

    phvwr          p.cqe.qid, CAPRI_RXDMA_INTRINSIC_QID
    phvwrpair      p.cqe.status[7:0], k.rdma_aq_feedback_status, p.cqe.error, k.rdma_aq_feedback_error
    phvwr          CAPRI_PHV_FIELD(AQCB_TO_CQ_P, cq_id), d.cq_id
    
    CAPRI_RESET_TABLE_2_ARG() //BD Slot


    phvwr       CAPRI_PHV_RANGE(AQCB_TO_CQ_P, cq_id, status), CAPRI_KEY_RANGE(rdma_aq_feedback,cq_num_sbit0_ebit7, status)
    phvwr       CAPRI_PHV_FIELD(AQCB_TO_CQ_P, error), k.rdma_aq_feedback_error

    add         r2, d.cqcb_addr, r0
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, rdma_aq_rx_cqcb_process, r2)

    //          populate LIF
    add            r1, r0, offsetof(struct phv_, common_global_global_data)    
    phvwr       CAPRI_PHV_FIELD(PHV_GLOBAL_COMMON_P, lif), CAPRI_RXDMA_INTRINSIC_LIF

    mfspr       r2, spr_tbladdr
    phvwr CAPRI_PHV_FIELD(TO_S6_INFO, aqcb_addr), r2[34:AQCB_ADDR_SHIFT]
    
    //set dma_cmd_ptr in phv
    RXDMA_DMA_CMD_PTR_SET(AQ_RX_DMA_CMD_START_FLIT_ID, AQ_RX_DMA_CMD_START_FLIT_CMD_ID) //Exit Slot

    nop.e
    nop

exit:
    phvwr.e       p.common.p4_intr_global_drop, 1   
    nop         //Exit Slot

create_qp: 

    phvwr       p.{rqcb0.intrinsic.host_rings, rqcb0.intrinsic.total_rings}, (MAX_RQ_RINGS<<4|MAX_RQ_RINGS)

    //          TODO: For now setting it to RTS, but later change it to INIT
    // state. modify_qp is supposed to set it to RTR and RTS.
    phvwr       p.rqcb0.state, QP_STATE_RTS

        //TODO: RQ in HBM still need to be implemented

    phvwr       p.rqcb0.log_rq_page_size, k.rdma_aq_feedback_qp_rq_page_size_log2[4:0]
    phvwrpair   p.rqcb0.log_wqe_size, k.rdma_aq_feedback_qp_rq_stride_log2[4:0], p.rqcb0.log_num_wqes , k.rdma_aq_feedback_qp_rq_depth_log2[4:0]
    phvwr       p.rqcb0.serv_type, k.rdma_aq_feedback_qp_rq_type_state

    //RQCB1

    phvwr       p.rqcb1.serv_type, k.rdma_aq_feedback_qp_rq_type_state
    phvwrpair   p.rqcb1.log_rq_page_size, k.rdma_aq_feedback_qp_rq_page_size_log2[4:0], p.rqcb1.state, QP_STATE_RTS 
    phvwrpair   p.rqcb1.log_wqe_size, k.rdma_aq_feedback_qp_rq_stride_log2[4:0], p.rqcb1.log_num_wqes , k.rdma_aq_feedback_qp_rq_depth_log2[4:0] 
    phvwrpair   p.rqcb1.serv_type, k.rdma_aq_feedback_qp_rq_type_state[2:0], p.rqcb1.pd, K_PD
    phvwr       p.rqcb1.cq_id, K_RQ_CQ_ID


    //RQCB2
    
    phvwrpair   p.rqcb2.rnr_timeout, 0xb, p.rqcb2.pd, K_PD

    //          TODO: Move RSQ/RRQ allocation to modify_qp frm create_qp
    //          TODO: Move pmtu setup to modify_qp
    
    //populate the PC in RQCB0, RQCB1
    addi        r4, r0, rdma_resp_rx_stage0[33:CAPRI_RAW_TABLE_PC_SHIFT] ;
    addi        r3, r0, rx_dummy[33:CAPRI_RAW_TABLE_PC_SHIFT] ;
    sub         r4, r4, r3
    phvwr       p.rqcb0.intrinsic.pc, r4
    phvwr       p.rqcb1.pc, r4


    /*
     * We do not have access to RQCB base address as well as PT base address, so
     * do the rest of the create_qp work (setting up DMA of RQCB & RQPT) in
     * stage1.
     */

    CAPRI_RESET_TABLE_3_ARG() 
    phvwrpair   CAPRI_PHV_FIELD(AQCB_TO_WQE_P, rq_id), K_RQ_ID, CAPRI_PHV_FIELD(AQCB_TO_WQE_P, rq_tbl_index), k.rdma_aq_feedback_qp_rq_tbl_index

    phvwr       CAPRI_PHV_FIELD(AQCB_TO_WQE_P, rq_map_count), K_RQ_MAP_COUNT
    phvwr       CAPRI_PHV_FIELD(AQCB_TO_WQE_P, rq_dma_addr), K_RQ_DMA_ADDR

    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, rdma_aq_rx_wqe_process, r0) 

    b           aq_feedback
    nop
    

#include "aq_rx.h"
#include "aqcb.h"

struct aq_rx_phv_t p;
struct aqcb0_t d;
struct common_p4plus_stage0_app_header_table_k k;

#define PHV_GLOBAL_COMMON_P phv_global_common

#define AQCB_TO_WQE_P t3_s2s_aqcb_to_wqe_info
#define AQCB_TO_SQCB_P t2_s2s_aqcb_to_sqcb1_info
    
#define TO_S4_INFO to_s4_info
#define TO_S5_INFO to_s5_info    
#define TO_S6_INFO to_s6_info

#define K_OP k.rdma_aq_feedback_op
#define K_WQE_ID k.{rdma_aq_feedback_wqe_id_sbit0_ebit7...rdma_aq_feedback_wqe_id_sbit8_ebit15} 
#define K_ERROR k.rdma_aq_feedback_error

//create_qp params    
#define K_CQP_PD k.{rdma_aq_feedback_cqp_pd_sbit0_ebit7...rdma_aq_feedback_cqp_pd_sbit16_ebit31}
#define K_CQP_RQ_MAP_COUNT k.{rdma_aq_feedback_cqp_rq_map_count_sbit0_ebit7...rdma_aq_feedback_cqp_rq_map_count_sbit16_ebit31}
#define K_CQP_RQ_DMA_ADDR k.{rdma_aq_feedback_cqp_rq_dma_addr_sbit0_ebit15...rdma_aq_feedback_cqp_rq_dma_addr_sbit56_ebit63}
#define K_CQP_RQ_CMB k.{rdma_aq_feedback_cqp_rq_cmb}
#define K_CQP_RQ_CQ_ID k.{rdma_aq_feedback_cqp_rq_cq_id_sbit0_ebit7...rdma_aq_feedback_cqp_rq_cq_id_sbit16_ebit23}
#define K_CQP_RQ_PRIVILEGED k.{rdma_aq_feedback_cqp_qp_privileged}
#define K_CQP_LOG_PMTU k.{rdma_aq_feedback_cqp_log_pmtu}
#define K_CQP_RQ_ID k.{rdma_aq_feedback_cqp_rq_id_sbit0_ebit15...rdma_aq_feedback_cqp_rq_id_sbit16_ebit23}
#define K_CQP_RQ_STRIDE_LOG2 k.{rdma_aq_feedback_cqp_rq_stride_log2}
#define K_CQP_RQ_DEPTH_LOG2 k.{rdma_aq_feedback_cqp_rq_depth_log2}

//modify_qp params
    
#define K_MQP_AH_LEN k.{rdma_aq_feedback_mqp_ah_len}
#define K_MQP_AH_ADDR k.{rdma_aq_feedback_mqp_ah_addr}
#define K_MQP_RRQ_BASE_ADDR k.{rdma_aq_feedback_mqp_rrq_base_addr_sbit0_ebit7...rdma_aq_feedback_mqp_rrq_base_addr_sbit8_ebit31}
#define K_MQP_RRQ_DEPTH_LOG2 k.{rdma_aq_feedback_mqp_rrq_depth_log2}
#define K_MQP_RSQ_BASE_ADDR k.{rdma_aq_feedback_mqp_rsq_base_addr_sbit0_ebit2...rdma_aq_feedback_mqp_rsq_base_addr_sbit27_ebit31}
#define K_MQP_RSQ_DEPTH_LOG2 k.{rdma_aq_feedback_mqp_rsq_depth_log2_sbit0_ebit2...rdma_aq_feedback_mqp_rsq_depth_log2_sbit3_ebit4}
#define K_MQP_AV_VALID k.{rdma_aq_feedback_mqp_av_valid}
#define K_MQP_RRQ_VALID k.{rdma_aq_feedback_mqp_rrq_valid}
#define K_MQP_RSQ_VALID k.{rdma_aq_feedback_mqp_rsq_valid}
#define K_MQP_STATE k.{rdma_aq_feedback_mqp_state}
#define K_MQP_STATE_VALID k.{rdma_aq_feedback_mqp_state_valid}
#define K_MQP_ERR_RETRY_CNT k.{rdma_aq_feedback_mqp_err_retry_count}
#define K_MQP_ERR_RETRY_CNT_VALID k.{rdma_aq_feedback_mqp_err_retry_count_valid}
#define K_MQP_TX_PSN k.{rdma_aq_feedback_mqp_tx_psn_sbit0_ebit1...rdma_aq_feedback_mqp_tx_psn_sbit18_ebit23}    
#define K_MQP_TX_PSN_VALID k.{rdma_aq_feedback_mqp_tx_psn_valid}
#define K_MQP_PMTU_LOG2 k.{rdma_aq_feedback_mqp_pmtu_log2_sbit0_ebit1...rdma_aq_feedback_mqp_pmtu_log2_sbit2_ebit4}    
#define K_MQP_PMTU_VALID k.{rdma_aq_feedback_mqp_pmtu_valid}
#define K_MQP_RQ_ID k.{rdma_aq_feedback_mqp_rq_id_sbit0_ebit6...rdma_aq_feedback_mqp_rq_id_sbit23_ebit23}
    
%%

    .param      rdma_aq_rx_cqcb_mpu_only_process
    .param      rdma_aq_rx_sqcb1_process
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
    nop

    phvwr       p.common.p4_intr_global_debug_trace, 1
    
    seq         c1, K_ERROR, r0
    bcf         [!c1], aq_feedback

    seq         c1, K_OP, AQ_OP_TYPE_CREATE_QP
    bcf         [c1], create_qp
    nop

    seq         c1, K_OP, AQ_OP_TYPE_MODIFY_QP
    bcf         [c1], modify_qp
    nop
    
aq_feedback:

    #CQCB is loaded by req_rx and resp_rx flows in stage-5, table-2 for mutual
    #exclusive access. Do the same for aq_rx also

    phvwrpair      p.cqe.admin.wqe_id, K_WQE_ID, p.cqe.admin.op_type, K_OP
    phvwr          p.cqe.qid, CAPRI_RXDMA_INTRINSIC_QID
    phvwrpair      p.cqe.status[7:0], k.rdma_aq_feedback_status, p.cqe.error, K_ERROR
    
    CAPRI_RESET_TABLE_0_ARG() //BD Slot

    phvwr       CAPRI_PHV_RANGE(TO_S6_INFO, wqe_id, status), CAPRI_KEY_RANGE(rdma_aq_feedback,wqe_id_sbit0_ebit7, status)
    phvwrpair   CAPRI_PHV_FIELD(TO_S6_INFO, cq_id), d.cq_id, CAPRI_PHV_FIELD(TO_S6_INFO, error), k.rdma_aq_feedback_error

    add         r2, d.cqcb_addr, r0
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, rdma_aq_rx_cqcb_mpu_only_process, r2)

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

    phvwr       p.{rqcb0.intrinsic.total_rings, rqcb0.intrinsic.host_rings}, (MAX_RQ_RINGS<<4|MAX_RQ_HOST_RINGS)

    //          TODO: For now setting it to RTS, but later change it to INIT
    // state. modify_qp is supposed to set it to RTR and RTS.
    phvwr       p.rqcb0.state, QP_STATE_RESET

        //TODO: RQ in HBM still need to be implemented

    phvwr       p.rqcb0.log_rq_page_size, k.rdma_aq_feedback_cqp_rq_page_size_log2[4:0]
    phvwrpair   p.rqcb0.log_wqe_size, K_CQP_RQ_STRIDE_LOG2[4:0], p.rqcb0.log_num_wqes , K_CQP_RQ_DEPTH_LOG2[4:0]
    phvwrpair   p.rqcb0.serv_type, k.rdma_aq_feedback_cqp_rq_type_state[2:0], p.rqcb0.log_pmtu, K_CQP_LOG_PMTU

    //RQCB1

    phvwrpair   p.rqcb1.log_rq_page_size, k.rdma_aq_feedback_cqp_rq_page_size_log2[4:0], p.rqcb1.state, QP_STATE_RESET
    phvwrpair   p.rqcb1.log_wqe_size, K_CQP_RQ_STRIDE_LOG2[4:0], p.rqcb1.log_num_wqes , K_CQP_RQ_DEPTH_LOG2[4:0]
    phvwrpair   p.rqcb1.serv_type, k.rdma_aq_feedback_cqp_rq_type_state[2:0], p.rqcb1.pd, K_CQP_PD
    phvwrpair   p.rqcb1.priv_oper_enable, K_CQP_RQ_PRIVILEGED, p.rqcb1.cq_id, K_CQP_RQ_CQ_ID
    phvwr       p.rqcb1.log_pmtu, K_CQP_LOG_PMTU

    //RQCB2
    
    phvwrpair   p.rqcb2.rnr_timeout, 0xb, p.rqcb2.pd, K_CQP_PD

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
    phvwrpair   CAPRI_PHV_FIELD(AQCB_TO_WQE_P, rq_id), K_CQP_RQ_ID, CAPRI_PHV_FIELD(AQCB_TO_WQE_P, rq_tbl_index), k.{rdma_aq_feedback_cqp_rq_tbl_index_sbit0_ebit7,rdma_aq_feedback_cqp_rq_tbl_index_sbit8_ebit31}

    phvwr       CAPRI_PHV_FIELD(AQCB_TO_WQE_P, rq_map_count), K_CQP_RQ_MAP_COUNT
    phvwr       CAPRI_PHV_FIELD(AQCB_TO_WQE_P, rq_dma_addr), K_CQP_RQ_DMA_ADDR
    phvwr       CAPRI_PHV_FIELD(AQCB_TO_WQE_P, rq_cmb), K_CQP_RQ_CMB

    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, rdma_aq_rx_wqe_process, r0) 

    b           aq_feedback
    nop
    
modify_qp:

    CAPRI_RESET_TABLE_2_ARG() 
    phvwr       CAPRI_PHV_RANGE(AQCB_TO_SQCB_P, ah_len, pmtu_valid), k.{rdma_aq_feedback_mqp_ah_len...rdma_aq_feedback_mqp_pmtu_valid}
    phvwr       CAPRI_PHV_FIELD(AQCB_TO_SQCB_P, rq_id), K_MQP_RQ_ID
    phvwr       CAPRI_PHV_RANGE(TO_S4_INFO, rrq_base_addr, rnr_retry_count_valid), k.{rdma_aq_feedback_mqp_rrq_base_addr_sbit0_ebit4...rdma_aq_feedback_mqp_rnr_retry_valid}
    phvwr       CAPRI_PHV_RANGE(TO_S5_INFO, rsq_base_addr, rsq_valid), k.{rdma_aq_feedback_mqp_rsq_base_addr_sbit0_ebit5...rdma_aq_feedback_mqp_rsq_valid}
    phvwr       CAPRI_PHV_RANGE(TO_S5_INFO, q_key, q_key_valid), k.{rdma_aq_feedback_mqp_q_key_sbit0_ebit15...rdma_aq_feedback_mqp_q_key_valid}
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, rdma_aq_rx_sqcb1_process, r0) 

    b           aq_feedback
    nop

    

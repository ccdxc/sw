#include "capri.h"
#include "aq_tx.h"
#include "aqcb.h"
#include "common_phv.h"
#include "p4/common/defines.h"
#include "types.h"

struct aq_tx_phv_t p;
struct aqwqe_t d;
struct aq_tx_s2_t1_k k;

#define IN_TO_S_P to_s2_info

#define TO_SQCB_INFO_P to_s3_info
    
#define WQE2_TO_SQCB0_P t1_s2s_wqe2_to_sqcb0_info
#define WQE2_TO_SQCB1_P t2_s2s_wqe2_to_sqcb1_info
#define WQE2_TO_SQCB2_P t3_s2s_wqe2_to_sqcb2_info

#define K_COMMON_GLOBAL_QID CAPRI_KEY_RANGE(phv_global_common, qid_sbit0_ebit4, qid_sbit21_ebit23)
#define K_COMMON_GLOBAL_QTYPE CAPRI_KEY_FIELD(phv_global_common, qtype)
#define K_AH_BASE_ADDR_PAGE_ID CAPRI_KEY_RANGE(IN_TO_S_P, ah_base_addr_page_id_sbit0_ebit15, ah_base_addr_page_id_sbit16_ebit21)
#define K_RRQ_BASE_ADDR_PAGE_ID CAPRI_KEY_RANGE(IN_TO_S_P, rrq_base_addr_page_id_sbit0_ebit1, rrq_base_addr_page_id_sbit18_ebit21)
#define K_RSQ_BASE_ADDR_PAGE_ID CAPRI_KEY_RANGE(IN_TO_S_P, rsq_base_addr_page_id_sbit0_ebit3, rsq_base_addr_page_id_sbit20_ebit21)
#define K_SQCB_BASE_ADDR_HI CAPRI_KEY_RANGE(IN_TO_S_P, sqcb_base_addr_hi_sbit0_ebit5, sqcb_base_addr_hi_sbit22_ebit23) 
#define K_RQCB_BASE_ADDR_HI CAPRI_KEY_RANGE(IN_TO_S_P, rqcb_base_addr_hi_sbit0_ebit5, rqcb_base_addr_hi_sbit22_ebit23)

%%

    .param      dummy
    .param      rdma_aq_tx_sqcb0_process
    .param      rdma_aq_tx_sqcb1_process
    .param      rdma_aq_tx_sqcb2_process
.align
rdma_aq_tx_modify_qp_2_process:

    add         r3, r0, d.{id_ver}.wx  //TODO: Need to optimize 
    SQCB_ADDR_GET(r1, r3[23:0], K_SQCB_BASE_ADDR_HI)
    RQCB_ADDR_GET(r2, r3[23:0], K_RQCB_BASE_ADDR_HI)

    CAPRI_RESET_TABLE_1_ARG()
    CAPRI_RESET_TABLE_2_ARG()
    CAPRI_RESET_TABLE_3_ARG()
    
hdr_update:
    bbne        d.mod_qp.attr_mask[RDMA_UPDATE_QP_OPER_SET_AV], 1, rrq_base
    
    add         r4, d.{mod_qp.ah_id_len}.wx, r0 //BD Slot
    add         r4, r4[23:0], r0
    muli        r5, r4, AT_ENTRY_SIZE_BYTES
    nop
    
    add         r5, r5, K_AH_BASE_ADDR_PAGE_ID, HBM_PAGE_SIZE_SHIFT
    srl         r7, r5, HDR_TEMP_ADDR_SHIFT

    //ah_len
    add         r4, d.{mod_qp.ah_id_len}.wx, r0 
    srl         r4, r4, 24

    phvwr       CAPRI_PHV_FIELD(WQE2_TO_SQCB0_P, av_valid), 1
    phvwr       CAPRI_PHV_FIELD(WQE2_TO_SQCB0_P, ah_len), r4
    phvwr       CAPRI_PHV_FIELD(WQE2_TO_SQCB0_P, ah_addr), r7

    phvwr       CAPRI_PHV_FIELD(WQE2_TO_SQCB1_P, av_valid), 1
    phvwr       CAPRI_PHV_FIELD(WQE2_TO_SQCB1_P, ah_len), r4
    phvwr       CAPRI_PHV_FIELD(WQE2_TO_SQCB1_P, ah_addr), r7

    phvwr       CAPRI_PHV_FIELD(WQE2_TO_SQCB2_P, av_valid), 1
    phvwr       CAPRI_PHV_FIELD(WQE2_TO_SQCB2_P, ah_len), r4
    phvwr       CAPRI_PHV_FIELD(WQE2_TO_SQCB2_P, ah_addr), r7
    
    //setup DMA in this stage but do CB updates in later stages.
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_MOD_QP_AH_SRC)
    or          r7, d.{mod_qp.dma_addr}.dx, 0x1, 63
    DMA_HOST_MEM2MEM_SRC_SETUP(r6, r4, r7)
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_MOD_QP_AH_DST)
    DMA_HBM_MEM2MEM_DST_SETUP(r6, r4, r5)
    
rrq_base:
    bbne        d.mod_qp.attr_mask[RDMA_UPDATE_QP_OPER_SET_MAX_QP_RD_ATOMIC], 1, rsq_base

    add         r7, r0, K_RRQ_BASE_ADDR_PAGE_ID, HBM_PAGE_SIZE_SHIFT //BD Slot
    add         r7, r7, r3[23:0], LOG_RRQ_QP_SIZE
    srl         r7, r7, RRQ_BASE_ADDR_SHIFT

    phvwr       CAPRI_PHV_FIELD(WQE2_TO_SQCB1_P, rrq_valid), 1
    phvwr       CAPRI_PHV_FIELD(WQE2_TO_SQCB1_P, rrq_depth_log2), d.mod_qp.rrq_depth[4: 0]
    phvwr       CAPRI_PHV_FIELD(WQE2_TO_SQCB1_P, rrq_base_addr), r7[31:0]

    phvwr       CAPRI_PHV_FIELD(WQE2_TO_SQCB2_P, rrq_valid), 1
    phvwr       CAPRI_PHV_FIELD(WQE2_TO_SQCB2_P, rrq_depth_log2), d.mod_qp.rrq_depth[4: 0]
    phvwr       CAPRI_PHV_FIELD(WQE2_TO_SQCB2_P, rrq_base_addr), r7[31:0]
    
rsq_base:
    bbne        d.mod_qp.attr_mask[RDMA_UPDATE_QP_OPER_SET_MAX_DEST_RD_ATOMIC], 1, state

    add         r7, r0, K_RSQ_BASE_ADDR_PAGE_ID, HBM_PAGE_SIZE_SHIFT //BD Slot
    add         r7, r7, r3[23:0], LOG_RSQ_QP_SIZE
    srl         r7, r7, RSQ_BASE_ADDR_SHIFT

    phvwr       CAPRI_PHV_FIELD(WQE2_TO_SQCB0_P, rsq_valid), 1
    phvwr       CAPRI_PHV_FIELD(WQE2_TO_SQCB0_P, rsq_depth_log2), d.mod_qp.rsq_depth[4: 0]
    phvwr       CAPRI_PHV_FIELD(WQE2_TO_SQCB0_P, rsq_base_addr), r7[31:0]

    phvwr       CAPRI_PHV_FIELD(WQE2_TO_SQCB1_P, rsq_valid), 1
    phvwr       CAPRI_PHV_FIELD(WQE2_TO_SQCB1_P, rsq_depth_log2), d.mod_qp.rsq_depth[4: 0]
    phvwr       CAPRI_PHV_FIELD(WQE2_TO_SQCB1_P, rsq_base_addr), r7[31:0]
    
state:

    bbne        d.mod_qp.attr_mask[RDMA_UPDATE_QP_OPER_SET_STATE], 1, tx_psn
    add         r2, d.type_state[2:0], r0
    
    //Need to translate ibv_state ionic_state
    .brbegin
     br          r2[2:0]    
     nop

    .brcase     IBV_QP_STATE_RESET
        add         r2, r0, QP_STATE_RESET
        b           state_next
        nop
    .brcase IBV_QP_STATE_INIT    
        add         r2, r0, QP_STATE_INIT
        b           state_next
        nop
    .brcase IBV_QP_STATE_RTR       
        add         r2, r0, QP_STATE_RTR
        b           state_next
        nop
    .brcase IBV_QP_STATE_RTS       
        add         r2, r0, QP_STATE_RTS
        b           state_next
        nop
    .brcase IBV_QP_STATE_SQD           
        add         r2, r0, QP_STATE_SQD
        b           state_next
        nop
    .brcase IBV_QP_STATE_SQ_ERR
        add         r2, r0, QP_STATE_SQ_ERR
        b           state_next
        nop
    .brcase IBV_QP_STATE_ERR                                            
        add         r2, r0, QP_STATE_ERR
        b           state_next
        nop
    .brcase IBV_QP_STATE_SQD_ON_ERR
        add         r2, r0, QP_STATE_SQD_ON_ERR
        b           state_next
        nop
    .brend

state_next:
    
    //State is only in SQCB0 and SQCB1
    phvwr       CAPRI_PHV_FIELD(WQE2_TO_SQCB0_P, state), r2[2:0]
    phvwr       CAPRI_PHV_FIELD(WQE2_TO_SQCB0_P, state_valid), 1
    phvwr       CAPRI_PHV_FIELD(WQE2_TO_SQCB1_P, state), r2[2:0]
    phvwr       CAPRI_PHV_FIELD(WQE2_TO_SQCB1_P, state_valid), 1

tx_psn:
    bbne        d.mod_qp.attr_mask[RDMA_UPDATE_QP_OPER_SET_SQ_PSN], 1, pmtu
    nop

    add         r4, d.{mod_qp.sq_psn}.wx, r0
    phvwr       CAPRI_PHV_FIELD(TO_SQCB_INFO_P, tx_psn), r4[23:0] 
    phvwr       CAPRI_PHV_FIELD(TO_SQCB_INFO_P, tx_psn_valid), 1

pmtu:

    bbne        d.mod_qp.attr_mask[RDMA_UPDATE_QP_OPER_SET_PATH_MTU], 1, setup_sqcb_stages
    nop
    
    phvwrpair       CAPRI_PHV_FIELD(WQE2_TO_SQCB0_P, pmtu_log2), d.mod_qp.pmtu[4:0], CAPRI_PHV_FIELD(WQE2_TO_SQCB0_P, pmtu_valid), 1
    phvwrpair       CAPRI_PHV_FIELD(WQE2_TO_SQCB1_P, pmtu_log2), d.mod_qp.pmtu[4:0], CAPRI_PHV_FIELD(WQE2_TO_SQCB1_P, pmtu_valid), 1

    
setup_sqcb_stages:

    phvwr       CAPRI_PHV_FIELD(WQE2_TO_SQCB0_P, qid), r3[23:0]
    phvwr       CAPRI_PHV_FIELD(WQE2_TO_SQCB1_P, qid), r3[23:0]
    phvwr       CAPRI_PHV_FIELD(WQE2_TO_SQCB2_P, qid), r3[23:0]
    
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, rdma_aq_tx_sqcb0_process, r1)
    add         r1, r1, CB_UNIT_SIZE_BYTES
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, rdma_aq_tx_sqcb1_process, r1)
    add         r1, r1, CB_UNIT_SIZE_BYTES
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, rdma_aq_tx_sqcb2_process, r1)

done:
    nop.e
    nop

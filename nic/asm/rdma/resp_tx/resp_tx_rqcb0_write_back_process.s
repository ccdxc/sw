#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"
#include "nic/p4/common/defines.h"

struct resp_tx_phv_t p;
struct resp_tx_s5_t1_k k;
struct rqcb0_t d;

#define DB_ADDR             r2
#define DB_DATA             r3
#define CURR_READ_RSP_PSN   r4
#define DMA_CMD_BASE        r1

#define IN_P t1_s2s_rqcb0_write_back_info
#define IN_TO_S_P to_s5_rqcb1_wb_info

%%

    .param  resp_tx_stats_process

.align
resp_tx_rqcb0_write_back_process:

    // invoke stats as mpu only
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_tx_stats_process, r0)

    bbeq       CAPRI_KEY_FIELD(IN_TO_S_P, flush_rq), 1, error_disable_qp_and_drop
    CAPRI_SET_TABLE_1_VALID(0) // BD slot

    // are we in a state to process received packets ?
    slt             c1, d.state, QP_STATE_RTR
    bcf             [c1], drop_phv

add_headers_common:
    // prior to adding any DMA commands to ship headers, check if it is UD service.
    seq             c1, d.serv_type, RDMA_SERV_TYPE_UD  //BD Slot
    bcf             [c1], process_ud

    // intrinsic
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_INTRINSIC) //BD Slot
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, common.p4_intr_global_tm_iport, common.p4_intr_global_tm_instance_type)

#ifndef GFT
    phvwrpair      p.common.p4_intr_global_tm_iport, TM_PORT_DMA, p.common.p4_intr_global_tm_oport, TM_PORT_INGRESS
#else
    phvwrpair      p.common.p4_intr_global_tm_iport, TM_PORT_DMA, p.common.p4_intr_global_tm_oport, TM_PORT_EGRESS
#endif

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_TXDMA_INTRINSIC)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, common.p4_txdma_intr_qid, common.p4_txdma_intr_txdma_rsv)
    phvwr          p.common.p4_txdma_intr_qid, K_GLOBAL_QID
    RQCB0_ADDR_GET(r1)
    phvwrpair       p.common.p4_txdma_intr_qstate_addr, r1, p.common.p4_txdma_intr_qtype, K_GLOBAL_QTYPE

    // common-p4+
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_COMMON_P4PLUS)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, p4plus_to_p4, p4plus_to_p4);
    phvwrpair       P4PLUS_TO_P4_APP_ID, P4PLUS_APPTYPE_RDMA, \
                     P4PLUS_TO_P4_FLAGS, (P4PLUS_TO_P4_FLAGS_UPDATE_IP_LEN | P4PLUS_TO_P4_FLAGS_UPDATE_UDP_LEN)

    bbeq           CAPRI_KEY_FIELD(IN_TO_S_P, ack_nak_process), 1, add_ack_header
    phvwr          P4PLUS_TO_P4_VLAN_TAG, 0 //BD-slot

rsq_write_back:
    // check if speculated psn matches with current psn
    seq         c1, CAPRI_KEY_FIELD(IN_P, curr_read_rsp_psn), d.curr_read_rsp_psn
    bcf         [!c1], drop_phv
    nop         // BD Slot

    bbeq        CAPRI_KEY_FIELD(IN_P, rate_enforce_failed), 1, dcqcn_rl_failure
    seq         c1, CAPRI_KEY_FIELD(IN_P, read_rsp_in_progress), 1 // BD Slot
    bcf         [c1], incr_psn
    tblwr       d.read_rsp_in_progress, CAPRI_KEY_FIELD(IN_P, read_rsp_in_progress) // BD Slot

    tblwr       d.curr_read_rsp_psn, 0
    // Update RSQ_C_INDEX to NEW_RSQ_C_INDEX only when read rsp NOT in progress (!c1)
    //TBD: do we need hx somewhere ?
    tblwr.e     RSQ_C_INDEX, CAPRI_KEY_FIELD(IN_TO_S_P, new_c_index)
    tblmincri   d.curr_color, 1, 1 // Exit Slot

incr_psn:
    // if read is in progress, incr curr_psn
    tblmincri.e  d.curr_read_rsp_psn, 24, 1
    nop // Exit Slot

    
add_ack_header:
    phvwr       CAPRI_PHV_FIELD(phv_global_common, _ack), 1

    add         r2, RDMA_PKT_OPC_ACK, d.serv_type, BTH_OPC_SVC_SHIFT
    phvwrpair   p.bth.opcode, r2, p.bth.dst_qp, d.dst_qp

    // phv_p->bth.pkey = 0xffff
    phvwr       p.bth.pkey, DEFAULT_PKEY
    
    // header_template
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_HDR_TEMPLATE)
    sll         r5, d.header_template_addr, HDR_TEMP_ADDR_SHIFT
    DMA_HBM_MEM2PKT_SETUP(DMA_CMD_BASE, d.header_template_size, r5)

    // BTH
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_BTH)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, bth, bth)

    // AETH
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_AETH)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, aeth, aeth)

    // No PAD for ack packet
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_ACK_ICRC)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, icrc, icrc)
    bbeq       K_GLOBAL_FLAG(_error_disable_qp), 1, error_disable_qp
    DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, DMA_CMD_BASE) //BD Slot

    DMA_SET_END_OF_CMDS_E(DMA_CMD_PHV2PKT_T, DMA_CMD_BASE)
    nop //Exit Slot
    
dcqcn_rl_failure:
    //toggle color so that S0 resets spec_psn to curr_psn
    tblmincri.e     d.curr_color, 1, 1
    nop

process_ud:
    // if error is encountered in RxDMA, we need to move txdma to error disable and inject a
    // feedback phv to SQ. Otherwise we can safely drop the phv as we don't need to send any ACK
    // in case of UD.
    bbeq        K_GLOBAL_FLAG(_error_disable_qp), 1, error_disable_qp
    nop         //BD Slot 
    // fall thru to drop_phv

drop_phv:
    phvwr.e         p.common.p4_intr_global_drop, 1
    nop // Exit Slot

error_disable_qp:
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, \
                            RESP_TX_DMA_CMD_RDMA_ERR_FEEDBACK)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, p4_intr_global, rdma_feedback)
    phvwrpair      p.p4_intr_global.tm_iport, TM_PORT_INGRESS, p.p4_intr_global.tm_oport, TM_PORT_DMA
    phvwrpair      p.p4_intr_global.tm_iq, 0, p.p4_intr_global.lif, K_GLOBAL_LIF
    phvwr          p.p4_intr_rxdma.intr_qid, K_GLOBAL_QID
    phvwri         p.p4_intr_rxdma.intr_rx_splitter_offset, RDMA_FEEDBACK_SPLITTER_OFFSET

    phvwrpair      p.p4_intr_rxdma.intr_qtype, Q_TYPE_RDMA_SQ, p.p4_to_p4plus.p4plus_app_id, P4PLUS_APPTYPE_RDMA
    phvwri         p.p4_to_p4plus.raw_flags, REQ_RX_FLAG_RDMA_FEEDBACK
    phvwri         p.{p4_to_p4plus.table0_valid...p4_to_p4plus.table1_valid}, 0x3

    phvwrpair      p.rdma_feedback.feedback_type, RDMA_COMPLETION_FEEDBACK, \
                   p.rdma_feedback.completion.status, CQ_STATUS_WQE_FLUSHED_ERR
    phvwrpair      p.rdma_feedback.completion.wrid, 0, \
                   p.rdma_feedback.completion.error, 1

    DMA_SET_END_OF_PKT_END_OF_CMDS_E(DMA_CMD_PHV2PKT_T, DMA_CMD_BASE)
    tblwr           d.state, QP_STATE_ERR   //Exit slot

error_disable_qp_and_drop:
    // don't execute any DMA instructions etc. all we do is set the state to ERROR and drop the phv.
    phvwr.e         p.common.p4_intr_global_drop, 1
    tblwr           d.state, QP_STATE_ERR   //Exit slot

    

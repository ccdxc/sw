#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"
#include "ingress.h"

struct resp_tx_phv_t p;
struct resp_tx_rqcb1_write_back_process_k_t k;
struct rqcb1_t d;

#define DB_ADDR             r2
#define DB_DATA             r3
#define CURR_READ_RSP_PSN   r4
#define DMA_CMD_BASE        r1

%%

resp_tx_rqcb1_write_back_process:

    bbeq       k.args.rate_enforce_failed, 1, dcqcn_rl_failure
    CAPRI_SET_TABLE_1_VALID(0) // BD slot


add_headers_common:

    // intrinsic
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_INTRINSIC)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, common.p4_intr_global_tm_iport, common.p4_intr_global_tm_instance_type)
    phvwrpair       p.common.p4_intr_global_tm_iport, TM_PORT_DMA, p.common.p4_intr_global_tm_oport, TM_PORT_INGRESS

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_TXDMA_INTRINSIC)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, common.p4_txdma_intr_qid, common.p4_txdma_intr_txdma_rsv)
    phvwr          p.common.p4_txdma_intr_qid, k.global.qid
    RQCB0_ADDR_GET(r1)
    phvwrpair       p.common.p4_txdma_intr_qstate_addr, r1, p.common.p4_txdma_intr_qtype, k.global.qtype

    // common-p4+
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_COMMON_P4PLUS)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, p4plus_to_p4, p4plus_to_p4);
    phvwrpair       P4PLUS_TO_P4_APP_ID, P4PLUS_APPTYPE_RDMA, P4PLUS_TO_P4_FLAGS, d.p4plus_to_p4_flags

    bbeq           k.to_stage.s5.rqcb1_wb.ack_nak_process, 1, add_ack_header
    phvwr          P4PLUS_TO_P4_VLAN_TAG, 0 //BD-slot

rsq_write_back:
    tblwr       d.read_rsp_in_progress, k.args.read_rsp_in_progress
    seq         c1, k.args.read_rsp_in_progress, 1
    cmov        CURR_READ_RSP_PSN, c1, k.args.curr_read_rsp_psn, 0
    add.c1      CURR_READ_RSP_PSN, CURR_READ_RSP_PSN, 1

    // TODO: ordering rules
    tblwr       d.curr_read_rsp_psn, CURR_READ_RSP_PSN
    tblwr       d.read_rsp_lock, 0
    bcf         [c1], exit

    // TODO: ordering rules
    // ring doorbell to update RSQ_C_INDEX to NEW_RSQ_C_INDEX
    // currently it is done thru memwr. Need to see if it should be done thru DMA
    DOORBELL_WRITE_CINDEX(k.global.lif, k.global.qtype, k.global.qid, RSQ_RING_ID, k.to_stage.s5.rqcb1_wb.new_c_index, DB_ADDR, DB_DATA)
    nop.e
    nop

    
add_ack_header:
    add         r2, RDMA_PKT_OPC_ACK, k.to_stage.s5.rqcb1_wb.ack_nack_serv_type, BTH_OPC_SVC_SHIFT
    phvwrpair   p.bth.opcode, r2, p.bth.dst_qp, d.dst_qp

    // prepare aeth
    phvwrpair   p.aeth.syndrome, d.aeth.syndrome, p.aeth.msn, d.aeth.msn

    // prepare BTH
    phvwr       p.bth.psn, d.ack_nak_psn
    tblwr       d.last_ack_nak_psn, d.ack_nak_psn

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

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_PAD_ICRC)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, icrc, icrc)

    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2PKT_T, DMA_CMD_BASE)
    DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, DMA_CMD_BASE)

    // TODO: ordering rules
    // ring doorbell to update C_INDEX to P_INDEX
    // currently it is done thru memwr. Need to see if it should be done thru DMA
    DOORBELL_WRITE_CINDEX(k.global.lif, k.global.qtype, k.global.qid, ACK_NAK_RING_ID, k.to_stage.s5.rqcb1_wb.new_c_index, DB_ADDR, DB_DATA)
    nop.e
    nop
    
dcqcn_rl_failure:
    bbeq           k.to_stage.s5.rqcb1_wb.ack_nak_process, 1, exit
    nop
    // release read_rsp_lock only in rsq path.
    tblwr       d.read_rsp_lock, 0

exit:
    nop.e
    nop

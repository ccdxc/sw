#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"
#include "ingress.h"

struct resp_tx_phv_t p;
struct resp_tx_ack_process_k_t k;
struct rqcb1_t d;

#define DMA_CMD_BASE        r1
#define DB_ADDR             r2
#define DB_DATA             r3
#define CURR_READ_RSP_PSN   r4
#define BTH_OPCODE          r5

%%

resp_tx_ack_process:

    // prepare BTH
    add         BTH_OPCODE, RDMA_PKT_OPC_ACK, k.args.serv_type, BTH_OPC_SVC_SHIFT
    phvwr       p.bth.opcode, BTH_OPCODE
    phvwr       p.bth.dst_qp, d.dst_qp
    phvwr       p.bth.psn, d.ack_nak_psn

    tblwr       d.last_ack_nak_psn, d.ack_nak_psn

    // prepare aeth
    phvwr       p.aeth.msn, d.aeth.msn
    phvwr       p.aeth.syndrome, d.aeth.syndrome

add_headers:
    // intrinsic
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_INTRINSIC)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, common.p4_intr_global_tm_iport, common.p4_intr_global_tm_instance_type)
    phvwri          p.common.p4_intr_global_tm_iport, TM_PORT_DMA
    phvwri          p.common.p4_intr_global_tm_oport, TM_PORT_INGRESS
    phvwri          p.common.p4_intr_global_tm_oq, 0

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_TXDMA_INTRINSIC)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, common.p4_txdma_intr_qid, common.p4_txdma_intr_txdma_rsv)
    phvwr           p.common.p4_txdma_intr_qid, k.global.qid
    RQCB0_ADDR_GET(r1)
    phvwr          p.common.p4_txdma_intr_qstate_addr, r1
    phvwr          p.common.p4_txdma_intr_qtype, k.global.qtype

    // common-p4+
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_COMMON_P4PLUS)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, p4plus_to_p4, p4plus_to_p4);
    phvwr          P4PLUS_TO_P4_APP_ID, P4PLUS_APPTYPE_RDMA
    phvwr          P4PLUS_TO_P4_FLAGS, d.p4plus_to_p4_flags
    phvwr          P4PLUS_TO_P4_VLAN_TAG, 0

    // header_template
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_HDR_TEMPLATE)
    DMA_HBM_MEM2PKT_SETUP(DMA_CMD_BASE, HDR_TEMPLATE_T_SIZE_BYTES, d.header_template_addr)

    // BTH
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_BTH)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, bth, bth)

    // AETH
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_AETH)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, aeth, aeth)

    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2PKT_T, DMA_CMD_BASE)
    DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, DMA_CMD_BASE)

    // TODO: ordering rules
    // ring doorbell to update ACK_NAK_C_INDEX to ACK_NAK_P_INDEX 
    // currently it is done thru memwr. Need to see if it should be done thru DMA
    DOORBELL_WRITE_CINDEX(k.global.lif, k.global.qtype, k.global.qid, ACK_NAK_RING_ID, k.args.new_c_index, DB_ADDR, DB_DATA) 

    CAPRI_SET_TABLE_0_VALID(0)
exit:
    nop.e
    nop

#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "defines.h"

struct req_tx_phv_t p;
struct req_tx_write_back_process_k_t k;
struct sqcb1_t d;

%%

.align
req_tx_add_headers_2_process:
    // get DMA cmd entry based on dma_cmd_index
    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_PHV_INTRINSIC)

    // dma_cmd[0] : addr1 - p4_intr
    DMA_PHV2PKT_SETUP_MULTI_ADDR_0(r6, common.p4_intr_global_tm_iport, common.p4_intr_global_tm_instance_type, 3)
    phvwri          p.common.p4_intr_global_tm_iport, TM_PORT_DMA
    phvwri          p.common.p4_intr_global_tm_oport, TM_PORT_INGRESS
    phvwri          p.common.p4_intr_global_tm_oq, 0

    // No need to fill p4_txdma_intr fields as they are already filled before stage0
    // dma_cmd[0] : addr2 - p4_txdma_intr
    DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, common.p4_txdma_intr_qid, common.p4_txdma_intr_txdma_rsv, 1)

    // dma_cmd[0] : addr3 - p4plus_to_p4_header
    DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, p4plus_to_p4, p4plus_to_p4, 2);
    phvwr          P4PLUS_TO_P4_APP_ID, P4PLUS_APPTYPE_RDMA
    phvwr          P4PLUS_TO_P4_FLAGS, d.p4plus_to_p4_flags

    #c3 - UD service. Needed only for send & send_imm
    seq            c3, d.service, RDMA_SERV_TYPE_UD

    //For UD, ah_handle comes in send req.
    cmov           r3, c3, k.args.op.send_wr.ah_handle, d.header_template_addr
    // dma_cmd[1] - header_template
    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_HEADER_TEMPLATE)
    DMA_HBM_MEM2PKT_SETUP(r6, HDR_TEMPLATE_T_SIZE_BYTES, r3)

    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RDMA_HEADERS)
    // dma_cmd[2] : addr2 - deth only if it is UD service (bth setup in add_headers_process)
    DMA_PHV2PKT_SETUP_MULTI_ADDR_N_C(r6, deth, deth, 1, c3)

    // phv_p->bth.dst_qp = sqcb1_p->dst_qp if it is not UD service
    phvwr.!c3      BTH_DST_QP, d.dst_qp

    CAPRI_SET_TABLE_3_VALID(0)

    bcf            [!c3], exit
    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RDMA_FEEDBACK) // Branch Delay Slot
    DMA_PHV2PKT_SETUP_MULTI_ADDR_0(r6, p4_intr_global, p4_to_p4plus, 2)
    DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, rdma_feedback, rdma_feedback, 1)
    add            r1, r0, offsetof(struct req_tx_phv_t, p4_to_p4plus)
    phvwrp         r1, 0, CAPRI_SIZEOF_RANGE(struct req_tx_phv_t, p4_intr_global, p4_to_p4plus), r0

    phvwr          p.p4_intr_global.tm_iport, TM_PORT_INGRESS
    phvwr          p.p4_intr_global.tm_oport, TM_PORT_DMA
    phvwr          p.p4_intr_global.tm_iq, 0
    phvwr          p.p4_intr_global.lif, k.global.lif     
    phvwr          p.p4_intr_rxdma.intr_qid, k.global.qid
    SQCB0_ADDR_GET(r1)
    phvwr          p.p4_intr_rxdma.intr_qstate_addr, r1
    phvwr          p.p4_intr_rxdma.intr_qtype, k.global.qtype
    phvwri         p.p4_intr_rxdma.intr_rx_splitter_offset, RDMA_FEEDBACK_SPLITTER_OFFSET 

    phvwr          p.p4_to_p4plus.p4plus_app_id, P4PLUS_APPTYPE_RDMA
    phvwri         p.p4_to_p4plus.raw_flags, REQ_RX_FLAG_RDMA_FEEDBACK

    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2PKT_T, r6)
    DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, r6)

exit:
    nop.e
    nop


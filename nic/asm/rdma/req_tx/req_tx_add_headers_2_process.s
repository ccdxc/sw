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
    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, 0)

    // dma_cmd[0] - p4_intr
    DMA_PHV2PKT_SETUP(r6, common.p4_intr_global_tm_iport, common.p4_intr_global_tm_instance_type)
    phvwri          p.common.p4_intr_global_tm_iport, TM_PORT_DMA
    phvwri          p.common.p4_intr_global_tm_oport, TM_PORT_INGRESS
    phvwri          p.common.p4_intr_global_tm_oq, 0

    // No need to fill p4_txdma_intr fields as they are already filled before stage0
    // dma_cmd[1] - p4_txdma_intr
    DMA_NEXT_CMD_I_BASE_GET(r6, 1)
    DMA_PHV2PKT_SETUP(r6, common.p4_txdma_intr_qid, common.p4_txdma_intr_txdma_rsv)

    // dma_cmd[2] - p4plus_to_p4_header
    DMA_NEXT_CMD_I_BASE_GET(r6, 1)
    DMA_PHV2PKT_SETUP(r6, p4plus_to_p4, p4plus_to_p4);
    phvwr          P4PLUS_TO_P4_APP_ID, P4PLUS_APPTYPE_RDMA
    phvwr          P4PLUS_TO_P4_FLAGS, d.p4plus_to_p4_flags

    #c3 - UD service. Needed only for send & send_imm
    seq            c3, d.service, RDMA_SERV_TYPE_UD

    // dma_cmd[3] - header_template
    DMA_NEXT_CMD_I_BASE_GET(r6, 1)

    //For UD, ah_handle comes in send req.
    cmov           r3, c3, k.args.op.send_wr.ah_handle, d.header_template_addr
    DMA_HBM_MEM2PKT_SETUP(r6, HDR_TEMPLATE_T_SIZE_BYTES, r3)

    // dma_cmd[4] - BTH
    addi           r6, r6, DMA_SWITCH_TO_NEXT_FLIT_BITS
    DMA_PHV2PKT_SETUP(r6, bth, bth)

    // dma_cmd[5] - deth only if it is UD service
    DMA_NEXT_CMD_I_BASE_GET(r6, 1)
    DMA_PHV2PKT_SETUP_C(r6, deth, deth, c3)

    // phv_p->bth.dst_qp = sqcb1_p->dst_qp if it is not UD service
    phvwr.!c3      BTH_DST_QP, d.dst_qp

    CAPRI_SET_TABLE_3_VALID(0)

    nop.e
    nop


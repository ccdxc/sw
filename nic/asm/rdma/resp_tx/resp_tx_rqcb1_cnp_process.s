#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"
#include "ingress.h"

struct resp_tx_phv_t p;
struct resp_tx_cnp_process_k_t k;
struct rqcb1_t d;

#define DMA_CMD_BASE        r1
#define BTH_OPCODE          r5
#define RAW_TABLE_PC        r3

%%

    .param      resp_tx_dcqcn_cnp_process

// Prepare CNP packet
resp_tx_rqcb1_cnp_process:

    // prepare BTH
    add         BTH_OPCODE, RDMA_PKT_OPC_CNP, RDMA_SERV_TYPE_CNP, BTH_OPC_SVC_SHIFT
    phvwr       p.bth.opcode, BTH_OPCODE
    phvwr       p.bth.dst_qp, d.dst_qp

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
    DMA_HBM_MEM2PKT_SETUP(DMA_CMD_BASE, d.header_template_size, d.header_template_addr)

    // BTH
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_BTH)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, bth, bth)
    
    // RSVD 16 bytes.
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_CNP_RSVD)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, cnp_rsvd, cnp_rsvd)

    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2PKT_T, DMA_CMD_BASE)
    DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, DMA_CMD_BASE)                                             

    // Load dcqcn_cb to fetch pkey to fill in BTH
    CAPRI_GET_TABLE_0_K(resp_tx_phv_t, r2)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_tx_dcqcn_cnp_process)
    add     r4, d.header_template_addr, HDR_TEMPLATE_T_SIZE_BYTES
    CAPRI_NEXT_TABLE_I_READ(r2, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, r4)

    nop.e
    nop

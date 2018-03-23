#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "common_phv.h"
#include "types.h" 

struct req_tx_phv_t p;
struct dcqcn_cb_t d;

#define DMA_CMD_BASE r1

%%

.align
req_tx_dcqcn_cnp_process:

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RDMA_HEADERS)
    // addr1 - BTH 
    DMA_PHV2PKT_SETUP_MULTI_ADDR_0(DMA_CMD_BASE, bth, bth, 2)
    // addr2 - RSVD 16 bytes.
    DMA_PHV2PKT_SETUP_MULTI_ADDR_N(DMA_CMD_BASE, cnp_rsvd, cnp_rsvd, 1)

    // Update BTH opcode
    phvwr       p.bth.opcode, RDMA_PKT_OPC_CNP
    // Update partition key in CNP packet
    phvwr       p.bth.pkey, 0xffff

    // For PAD and ICRC
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RDMA_PAD_ICRC)
    // dma_cmd[0] : addr1 - pad/icrc
    DMA_PHV2PKT_SETUP_MULTI_ADDR_0(DMA_CMD_BASE, immeth, immeth, 1)

    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2PKT_T, DMA_CMD_BASE)
    DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, DMA_CMD_BASE)

exit:
    CAPRI_SET_TABLE_0_VALID(0)
    nop.e
    nop

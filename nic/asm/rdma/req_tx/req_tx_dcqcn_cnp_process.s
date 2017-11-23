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
    add         r5, RDMA_PKT_OPC_CNP, RDMA_SERV_TYPE_CNP, BTH_OPC_SVC_SHIFT
    phvwr       p.bth.opcode, r5
    // Update partition key in CNP packet
    phvwr       p.bth.pkey, d.partition_key

    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2PKT_T, DMA_CMD_BASE)
    DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, DMA_CMD_BASE)
exit:
    CAPRI_SET_TABLE_0_VALID(0)
    nop.e
    nop

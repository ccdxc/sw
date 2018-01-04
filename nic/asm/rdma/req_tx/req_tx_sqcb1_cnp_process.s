#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "types.h"
#include "common_phv.h"
#include "ingress.h"
#include "defines.h"

struct req_tx_phv_t p;
struct sqcb1_t d;

#define DMA_CMD_BASE        r1
#define RAW_TABLE_PC        r3

%%

    .param      req_tx_dcqcn_cnp_process

// Prepare CNP packet
req_tx_sqcb1_cnp_process:
    
    // Update dst_qp of the packet.
    phvwr       p.bth.dst_qp, d.dst_qp

add_headers:

    // get DMA cmd entry based on dma_cmd_index
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_PHV_INTRINSIC)

    // dma_cmd[0] : addr1 - p4_intr
    DMA_PHV2PKT_SETUP_MULTI_ADDR_0(DMA_CMD_BASE, common.p4_intr_global_tm_iport, common.p4_intr_global_tm_instance_type, 3)
    phvwrpair       p.common.p4_intr_global_tm_iport, TM_PORT_DMA, p.common.p4_intr_global_tm_oport, TM_PORT_INGRESS


    // No need to fill p4_txdma_intr fields as they are already filled before stage0
    // dma_cmd[0] : addr2 - p4_txdma_intr
    DMA_PHV2PKT_SETUP_MULTI_ADDR_N(DMA_CMD_BASE, common.p4_txdma_intr_qid, common.p4_txdma_intr_txdma_rsv, 1)

    // dma_cmd[0] : addr3 - p4plus_to_p4_header
    DMA_PHV2PKT_SETUP_MULTI_ADDR_N(DMA_CMD_BASE, p4plus_to_p4, p4plus_to_p4, 2);
    phvwrpair        P4PLUS_TO_P4_APP_ID, P4PLUS_APPTYPE_RDMA, P4PLUS_TO_P4_FLAGS, d.p4plus_to_p4_flags

    // dma_cmd[1] - header_template
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_HEADER_TEMPLATE)
    sll            r4, d.header_template_addr, HDR_TEMP_ADDR_SHIFT
    DMA_HBM_MEM2PKT_SETUP(DMA_CMD_BASE, d.header_template_size, r4)

    // Load dcqcn_cb to fetch pkey to fill in BTH
    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r2)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, req_tx_dcqcn_cnp_process)
    add     r4, r4, HDR_TEMPLATE_T_SIZE_BYTES
    CAPRI_NEXT_TABLE_I_READ(r2, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, r4)

    nop.e
    nop

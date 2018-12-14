#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "common_phv.h"
#include "defines.h"
#include "capri-macros.h"


struct req_tx_phv_t p;
struct sqcb2_t d;
struct req_tx_s1_t0_k k;

#define DMA_CMD_BASE        r1

%%

    .param      req_tx_dcqcn_cnp_process
    .param      lif_stats_base

// Prepare CNP packet
req_tx_sqcb2_cnp_process:
    
    // Update dst_qp of the packet.
    phvwr       p.bth.dst_qp, d.dst_qp
    // Mellanox expects BECN to be set in CNP packet though RoceV2-Annex doesn't mandate. 
    // Set BECN bit for now for interop with Mellanox.
    phvwr       p.bth.b, 1

add_headers:

    // get DMA cmd entry based on dma_cmd_index
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_PHV_INTRINSIC)

    // dma_cmd[0] : addr1 - p4_intr
    DMA_PHV2PKT_SETUP_MULTI_ADDR_0(DMA_CMD_BASE, common.p4_intr_global_tm_iport, common.p4_intr_global_tm_instance_type, 3)

#ifndef GFT
    phvwrpair      p.common.p4_intr_global_tm_iport, TM_PORT_DMA, p.common.p4_intr_global_tm_oport, TM_PORT_INGRESS
#else
    phvwrpair      p.common.p4_intr_global_tm_iport, TM_PORT_DMA, p.common.p4_intr_global_tm_oport, TM_PORT_EGRESS
#endif


    // No need to fill p4_txdma_intr fields as they are already filled before stage0
    // dma_cmd[0] : addr2 - p4_txdma_intr
    DMA_PHV2PKT_SETUP_MULTI_ADDR_N(DMA_CMD_BASE, common.p4_txdma_intr_qid, common.p4_txdma_intr_txdma_rsv, 1)

    // dma_cmd[0] : addr3 - p4plus_to_p4_header
    DMA_PHV2PKT_SETUP_MULTI_ADDR_N(DMA_CMD_BASE, p4plus_to_p4, p4plus_to_p4, 2);
    phvwrpair        P4PLUS_TO_P4_APP_ID, P4PLUS_APPTYPE_RDMA, \
                     P4PLUS_TO_P4_FLAGS, (P4PLUS_TO_P4_FLAGS_UPDATE_IP_LEN | P4PLUS_TO_P4_FLAGS_UPDATE_UDP_LEN) 

    // dma_cmd[1] - header_template
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_HEADER_TEMPLATE)
    sll            r4, d.header_template_addr, HDR_TEMP_ADDR_SHIFT
    DMA_HBM_MEM2PKT_SETUP(DMA_CMD_BASE, d.header_template_size, r4)


    // CNP RDMA Headers - BTH + Rsvd bytes.
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RDMA_HEADERS)
    // addr1 - BTH 
    DMA_PHV2PKT_SETUP_MULTI_ADDR_0(DMA_CMD_BASE, bth, bth, 2)
    // addr2 - RSVD 16 bytes.
    DMA_PHV2PKT_SETUP_MULTI_ADDR_N(DMA_CMD_BASE, cnp_rsvd, cnp_rsvd, 1)

    // Update BTH opcode
    phvwr       p.bth.opcode, RDMA_PKT_OPC_CNP
    // Update partition key in CNP packet
    phvwr       p.bth.pkey, DEFAULT_PKEY

    // For PAD and ICRC
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RDMA_PAD_ICRC)
    // dma_cmd[0] : addr1 - pad/icrc
    DMA_PHV2PKT_SETUP_MULTI_ADDR_0(DMA_CMD_BASE, immeth, immeth, 1)
    
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2PKT_T, DMA_CMD_BASE)
    DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, DMA_CMD_BASE)

    CAPRI_SET_TABLE_0_VALID(0)

handle_lif_stats:

#ifndef GFT

    addi            r1, r0, CAPRI_MEM_SEM_ATOMIC_ADD_START
    addi            r2, r0, lif_stats_base[30:0] // substract 0x80000000 because hw adds it
    add             r2, r2, K_GLOBAL_LIF, LIF_STATS_SIZE_SHIFT

    #tx cnp packets
    addi            r3, r2, LIF_STATS_TX_RDMA_CNP_PACKETS_OFFSET

    ATOMIC_INC_VAL_1(r1, r3, r4, r5, 1)

#endif

exit:
    nop.e
    nop

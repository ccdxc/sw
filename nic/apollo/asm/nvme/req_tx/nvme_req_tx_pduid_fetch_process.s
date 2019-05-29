#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s5_t2_nvme_req_tx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s5_t2_nvme_req_tx_k_ k;
struct s5_t2_nvme_req_tx_pduid_fetch_process_d d;

#define DMA_CMD_BASE    r5
#define PDU_CTXT_P      r6
#define TMP_R           r7

%%
    .param  nvme_tx_pdu_context_base

.align
nvme_req_tx_pduid_fetch_process:
    
    addui   PDU_CTXT_P, r0, hiword(nvme_tx_pdu_context_base)
    addi    PDU_CTXT_P, PDU_CTXT_P, loword(nvme_tx_pdu_context_base)
    add     PDU_CTXT_P, PDU_CTXT_P, PDUID, LOG_PDU_CTXT_SIZE

    DMA_CMD_BASE_GET(DMA_CMD_BASE, pdu_ctxt0_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, pdu_ctxt0_cmd_opc, pdu_ctxt0_pad, PDU_CTXT_P)

    bbeq    k.to_s5_info_prp1_dma_valid, 0, exit
    add     PDU_CTXT_P, PDU_CTXT_P, NVME_PDU_CTXT_PRP_LIST_OFFSET   //BD Slot

    DMA_CMD_BASE_GET(DMA_CMD_BASE, prp1_dma)
    DMA_HBM_PHV2MEM_START_LEN_SETUP(DMA_CMD_BASE, TMP_R, prp1_ptr, k.to_s5_info_prp1_dma_bytes, PDU_CTXT_P)

    bbeq    k.to_s5_info_prp2_dma_valid, 0, exit
    add     PDU_CTXT_P, PDU_CTXT_P, k.to_s5_info_prp1_dma_bytes     //BD Slot

    DMA_CMD_BASE_GET(DMA_CMD_BASE, prp2_dst_dma)
    DMA_HBM_MEM2MEM_DST_SETUP(DMA_CMD_BASE, k.to_s5_info_prp2_dma_bytes, PDU_CTXT_P)
    
    bbeq    k.to_s5_info_prp3_dma_valid, 0, exit
    add     PDU_CTXT_P, PDU_CTXT_P, k.to_s5_info_prp2_dma_bytes     //BD Slot

    DMA_CMD_BASE_GET(DMA_CMD_BASE, prp3_dst_dma)
    DMA_HBM_MEM2MEM_DST_SETUP(DMA_CMD_BASE, k.to_s5_info_prp3_dma_bytes, PDU_CTXT_P)
    
exit:
    phvwr.e     p.sess_wqe_pduid, PDUID
    CAPRI_SET_TABLE_2_VALID(0)      //Exit Slot

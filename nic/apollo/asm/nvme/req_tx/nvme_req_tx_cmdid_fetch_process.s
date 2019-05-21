#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s5_t1_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s5_t1_k_ k;
struct s5_t1_nvme_req_tx_cmdid_fetch_process_d d;

#define DMA_CMD_BASE    r5
#define CMD_CTXT_P      r6
#define TMP_R           r7

%%
    .param  nvme_cmd_context_base

.align
nvme_req_tx_cmdid_fetch_process:

    addui   CMD_CTXT_P, r0, hiword(nvme_cmd_context_base)
    addi    CMD_CTXT_P, CMD_CTXT_P, loword(nvme_cmd_context_base)
    add     CMD_CTXT_P, CMD_CTXT_P, CMDID, LOG_CMD_CTXT_SIZE

    DMA_CMD_BASE_GET(DMA_CMD_BASE, cmd_ctxt_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, cmd_ctxt_opc, cmd_ctxt_pad, CMD_CTXT_P)

    bbeq    k.to_s5_info_prp1_dma_valid, 0, exit
    add     CMD_CTXT_P, CMD_CTXT_P, NVME_CMD_CTXT_PRP_LIST_OFFSET   //BD Slot

    DMA_CMD_BASE_GET(DMA_CMD_BASE, prp1_dma)
    DMA_HBM_PHV2MEM_START_LEN_SETUP(DMA_CMD_BASE, TMP_R, prp1_ptr, k.to_s5_info_prp1_dma_bytes, CMD_CTXT_P)

    bbeq    k.to_s5_info_prp2_dma_valid, 0, exit
    add     CMD_CTXT_P, CMD_CTXT_P, k.to_s5_info_prp1_dma_bytes     //BD Slot

    DMA_CMD_BASE_GET(DMA_CMD_BASE, prp2_dst_dma)
    DMA_HBM_MEM2MEM_DST_SETUP(DMA_CMD_BASE, k.to_s5_info_prp2_dma_bytes, CMD_CTXT_P)
    
    bbeq    k.to_s5_info_prp3_dma_valid, 0, exit
    add     CMD_CTXT_P, CMD_CTXT_P, k.to_s5_info_prp2_dma_bytes     //BD Slot

    DMA_CMD_BASE_GET(DMA_CMD_BASE, prp3_dst_dma)
    DMA_HBM_MEM2MEM_DST_SETUP(DMA_CMD_BASE, k.to_s5_info_prp3_dma_bytes, CMD_CTXT_P)
    
exit:
    phvwr.e     p.sess_wqe_cid, CMDID
    CAPRI_SET_TABLE_1_VALID(0)      //Exit Slot

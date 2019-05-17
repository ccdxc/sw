#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s3_t2_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s3_t2_k_ k;
struct s3_t2_nvme_req_tx_sqe_prp_process_d d;

#define DMA_CMD_BASE    r7

%%

.align
nvme_req_tx_sqe_prp_process:
    DMA_CMD_BASE_GET(DMA_CMD_BASE, prp3_src_dma)
    DMA_HOST_MEM2MEM_SRC_SETUP(DMA_CMD_BASE, k.t2_s2s_nscb_to_sqe_prp_info_prp3_dma_bytes, d.{ptr}.dx)

    nop.e
    CAPRI_SET_TABLE_2_VALID(0)      //Exit Slot


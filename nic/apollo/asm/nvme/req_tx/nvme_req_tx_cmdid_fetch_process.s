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

%%
    .param  nvme_cmd_context_base

.align
nvme_req_tx_cmdid_fetch_process:

    addui   CMD_CTXT_P, r0, hiword(nvme_cmd_context_base)
    addi    CMD_CTXT_P, CMD_CTXT_P, loword(nvme_cmd_context_base)
    add     CMD_CTXT_P, CMD_CTXT_P, CMDID, LOG_CMD_CTXT_SIZE
    
    //store backend command id so that PDU is sent with this CID
    phvwr   p.pdu_ctxt0_cid, CMDID

    DMA_CMD_BASE_GET(DMA_CMD_BASE, cmd_ctxt_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, cmd_ctxt_opc, cmd_ctxt_pad, CMD_CTXT_P)

exit:
    phvwr.e     p.sess_wqe_cmdid, CMDID
    CAPRI_SET_TABLE_1_VALID(0)      //Exit Slot

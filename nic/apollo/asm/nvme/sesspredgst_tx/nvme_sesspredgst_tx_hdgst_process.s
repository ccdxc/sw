#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s5_t0_nvme_sesspredgst_tx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s5_t0_nvme_sesspredgst_tx_k_ k;
struct s5_t0_nvme_sesspredgst_tx_hdgst_process_d d;

#define DMA_CMD_BASE   r7

%%

.align
nvme_sesspredgst_tx_hdgst_process:
    // setup hdgst_desc
    //add            r2, r0, r0
    //tblrdp         r1, r2, offsetof(struct nvme_cmd_ctxt_page_entry_t, page_addr), \
    //               sizeof(struct nvme_cmd_ctxt_page_entry_t.page_addr)
    add            r1, d.page0, PKT_DESC_OVERHEAD
    phvwr          p.hdgst_desc_src, r1.dx

    // DMA cmd to write PDU CH and PSH hdr in tcp page
    DMA_CMD_BASE_GET(DMA_CMD_BASE, pdu_hdr_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, pdu_ch_pdu_type, pdu_psh_dw15, r1)

    // PDU hdr size without including hdgst size
    add            r1, NVME_O_TCP_CMD_CAPSULE_CH_PSH_SIZE, r0
    or             r2, HW_CMD_HDGST, r1, 16
    phvwr          p.hdgst_desc_cmd, r2.dx

    // Set SGL1 addr to start of command data buffer
    add            r1, r1, NVME_O_TCP_HDGST_SIZE
    phvwr.e        p.pdu_psh_sgl1_addr, r1.dx
    CAPRI_SET_TABLE_0_VALID(0) // Exit Slot

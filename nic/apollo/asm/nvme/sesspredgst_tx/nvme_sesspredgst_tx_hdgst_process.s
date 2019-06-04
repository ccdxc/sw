#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s5_t0_nvme_sesspredgst_tx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s5_t0_nvme_sesspredgst_tx_k_ k;
struct s5_t0_nvme_sesspredgst_tx_hdgst_process_d d;

%%

.align
nvme_sesspredgst_tx_hdgst_process:
    // setup hdgst_desc
    //add            r2, r0, r0
    //tblrdp         r1, r2, offsetof(struct nvme_cmd_ctxt_page_entry_t, page_addr), \
    //               sizeof(struct nvme_cmd_ctxt_page_entry_t.page_addr)
    add            r1, d.page0, TCP_PAGE_NVME_O_TCP_CH_OFFSET
    phvwr          p.hdgst_desc_src, r1.dx
    // PDU hdr size without including hdgst size
    add            r1, NVME_O_TCP_CMD_CAPSULE_CH_PSH_SIZE, r0
    or             r1, HW_CMD_HDGST, r1, 16
    phvwr.e        p.hdgst_desc_cmd, r1.dx

    CAPRI_SET_TABLE_0_VALID(0) // Exit Slot

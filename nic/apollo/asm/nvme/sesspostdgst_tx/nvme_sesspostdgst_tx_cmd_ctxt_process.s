#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s2_t0_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s2_t0_k_ k;
struct s2_t0_nvme_sesspostdgst_tx_cmd_ctxt_process_d d;

#define SESSPRODCB_P r1
%%
    .param    nvme_txsessprodcb_base
    .param    nvme_sesspostdgst_tx_sessprodcb_process

.align
nvme_sesspostdgst_tx_cmd_ctxt_process:
    // pass num_pages to sessprodcb_process to compute number of sesq ring
    // entries required to send this PDU and to sesspostdgst_tx_cb_writeback
    // to trigger one or two tso_process depending on the total number of
    // pages to post to TCP
    phvwr          p.to_s3_info_num_pages, d.num_pages
    phvwr          p.to_s4_info_num_pages, d.num_pages

    addui          SESSPRODCB_P, r0, hiword(nvme_txsessprodcb_base)
    addi           SESSPRODCB_P, SESSPRODCB_P, loword(nvme_txsessprodcb_base)
    add            SESSPRODCB_P, SESSPRODCB_P, d.session_id, LOG_TXSESSPRODCB_SIZE

    // load sessprodcb_process to get TCP sesq pi and ci and
    // to increment tcp sesq pi by num_pages if sufficient entries are
    // available to post
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_EN,
                                CAPRI_TABLE_SIZE_512_BITS,
                                nvme_sesspostdgst_tx_sessprodcb_process,
                                SESSPRODCB_P) // Exit Slot

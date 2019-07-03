#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s3_t0_nvme_sesspostdgst_tx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s3_t0_nvme_sesspostdgst_tx_k_ k;
struct s3_t0_nvme_sesspostdgst_tx_pdu_ctxt0_process_d d;

#define SESSPRODTXCB_P r1
%%
    .param    nvme_txsessprodcb_base
    .param    nvme_sesspostdgst_tx_sessprodtxcb_process

.align
nvme_sesspostdgst_tx_pdu_ctxt0_process:
    // pass num_pages to sessprodtxcb_process to compute number of sesq ring
    // entries required to send this PDU and to sesspostdgst_tx_cb_writeback
    // to trigger one or two tso_process depending on the total number of
    // pages to post to TCP
    phvwr          p.to_s4_info_num_pages, d.num_pages
    phvwr          p.to_s5_info_num_pages, d.num_pages
    phvwr          p.to_s6_info_total_pages, d.num_pages

    addui          SESSPRODTXCB_P, r0, hiword(nvme_txsessprodcb_base)
    addi           SESSPRODTXCB_P, SESSPRODTXCB_P, loword(nvme_txsessprodcb_base)
    add            SESSPRODTXCB_P, SESSPRODTXCB_P, d.session_id, LOG_TXSESSPRODCB_SIZE

    // load sessprodtxcb_process to get TCP sesq pi and ci and
    // to increment tcp sesq pi by num_pages if sufficient entries are
    // available to post
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_EN,
                                CAPRI_TABLE_SIZE_512_BITS,
                                nvme_sesspostdgst_tx_sessprodtxcb_process,
                                SESSPRODTXCB_P) // Exit Slot

#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s1_t0_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s1_t0_k_ k;
struct s1_t0_nvme_sesspostdgst_tx_sess_wqe_process_d d;

#define CMD_CTXT_P   r1

%%
    .param    nvme_sesspostdgst_tx_cmd_ctxt_process
    .param    nvme_cmd_context_base

.align
nvme_sesspostdgst_tx_sess_wqe_process:

    // calculate cmd_ctxt address
    // nvme_cmd_context_basee + (cmd_id << sizeof(cmd_context))
    addui          CMD_CTXT_P, r0, hiword(nvme_cmd_context_base)
    addi           CMD_CTXT_P, CMD_CTXT_P, loword(nvme_cmd_context_base)
    add            CMD_CTXT_P, CMD_CTXT_P, d.cid, LOG_CMD_CTXT_SIZE

    // Pass cid to sessdgsttx_cb_writeback to load tso_process
    // starting from first tcp_page
    phvwr          p.to_s4_info_cid, d.cid

    // load cmd_ctx process to get nlb, number of pages
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS,
                                CAPRI_TABLE_SIZE_512_BITS,
                                nvme_sesspostdgst_tx_cmd_ctxt_process,
                                CMD_CTXT_P) // Exit Slot

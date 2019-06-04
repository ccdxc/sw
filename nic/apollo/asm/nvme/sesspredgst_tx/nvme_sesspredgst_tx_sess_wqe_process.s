#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s1_t0_nvme_sesspredgst_tx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s1_t0_nvme_sesspredgst_tx_k_ k;
struct s1_t0_nvme_sesspredgst_tx_sess_wqe_process_d d;

#define PDU_CTXT_P   r1

%%
    .param    nvme_sesspredgst_tx_pdu_ctxt_process
    .param    nvme_tx_pdu_context_base

.align
nvme_sesspredgst_tx_sess_wqe_process:

    // calculate pdu_ctxt address
    // nvme_tx_pdu_context_basee + (pdu_id << sizeof(pdu_context))
    addui          PDU_CTXT_P, r0, hiword(nvme_tx_pdu_context_base)
    addi           PDU_CTXT_P, PDU_CTXT_P, loword(nvme_tx_pdu_context_base)
    add            PDU_CTXT_P, PDU_CTXT_P, d.pduid, LOG_PDU_CTXT_SIZE

    // ddgst and hdgst status and computed integrity tag are stored
    // in pdu_ctxt which is then verified by sesspostdgst_tx and filled
    // in the pdu. Store the ddgst_desc and hdgst_desc status addresses
    // over here, as pdu_ctxt address is available in this program

    // setup ddgst_desc status addr and data
    add            r2, PDU_CTXT_P, NVME_PDU_CTXT_DDGST_STATUS_OFFSET
    phvwr          p.ddgst_desc_status_addr, r2.dx
    phvwr          p.ddgst_desc_status_data, 0 // TODO need to figure out status data

    // setup hdgst_desc status addr and data
    add            r2, PDU_CTXT_P, NVME_PDU_CTXT_HDGST_STATUS_OFFSET
    phvwr          p.hdgst_desc_status_addr, r2.dx
    phvwr          p.hdgst_desc_status_data, 0 // TODO need to figure out status data

    // Pass pduid to sesspredgst_tx_cb_writeback to load pdu_ctxt
    // page list for retrieving page_addr pointing to start of pdu
    phvwr          p.to_s4_info_pduid, d.pduid

    // load pdu_ctx process to get nlb, number of pages
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS,
                                CAPRI_TABLE_SIZE_512_BITS,
                                nvme_sesspredgst_tx_pdu_ctxt_process,
                                PDU_CTXT_P) // Exit Slot



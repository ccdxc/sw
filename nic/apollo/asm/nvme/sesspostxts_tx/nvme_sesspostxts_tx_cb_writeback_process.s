#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s6_t0_nvme_sesspostxts_tx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s6_t0_nvme_sesspostxts_tx_k_ k;
struct s6_t0_nvme_sesspostxts_tx_cb_writeback_process_d d;

#define PDU_CTXT_P r1
%%
    .param    nvme_tx_pdu_context_base
    .param    nvme_sesspostxts_tx_pdu_ctxt_process

.align
nvme_sesspostxts_tx_cb_writeback_process:

    tblmincri       SESSPOSTXTSTX_C_INDEX, d.log_num_entries, 1
    tblmincri.f     d.wb_r1_busy, 1, 1  //Flush

    // calculate pdu_ctxt address
    // nvme_tx_pdu_context_basee + (pdu_id << sizeof(pdu_context))
    addui          PDU_CTXT_P, r0, hiword(nvme_tx_pdu_context_base)
    addi           PDU_CTXT_P, PDU_CTXT_P, loword(nvme_tx_pdu_context_base)
    add            PDU_CTXT_P, PDU_CTXT_P, k.to_s6_info_pduid, LOG_PDU_CTXT_SIZE

    // load pdu_ctx process to get nlb, number of pages
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS,
                                CAPRI_TABLE_SIZE_512_BITS,
                                nvme_sesspostxts_tx_pdu_ctxt_process,
                                PDU_CTXT_P) // Exit Slot

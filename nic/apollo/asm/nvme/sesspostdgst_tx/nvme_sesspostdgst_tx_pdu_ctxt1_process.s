#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s2_t0_nvme_sesspostdgst_tx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s2_t0_nvme_sesspostdgst_tx_k_ k;
struct s2_t0_nvme_sesspostdgst_tx_pdu_ctxt1_process_d d;

#define SESSDGST_CB_P r1
#define PDU_CTXT0_P   r2
%%
    .param    nvme_sesspostdgst_tx_cb_writeback_process
    .param    nvme_sesspostdgst_tx_pdu_ctxt0_process

.align
nvme_sesspostdgst_tx_pdu_ctxt1_process:
    add            r1, d.{hdgst_status}.dx, r0
    seq            c1, r1[HW_DGST_STATUS_VALID_BIT:HW_DGST_STATUS_ENC_ERR_START_BIT], HW_DGST_STATUS_SUCCESS
    bcf            [!c1], exit

    add            r1, d.{ddgst_status}.dx, r0 // BD Slot
    seq            c1, r1[HW_DGST_STATUS_VALID_BIT:HW_DGST_STATUS_ENC_ERR_START_BIT], HW_DGST_STATUS_SUCCESS
    bcf            [!c1], exit
    mfspr          PDU_CTXT0_P, spr_tbladdr  // BD Slot
    sub            PDU_CTXT0_P, PDU_CTXT0_P, NVME_PDU_CTXT1_SIZE

    // load cmd_ctx process to get nlb, number of pages
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS,
                                CAPRI_TABLE_SIZE_512_BITS,
                                nvme_sesspostdgst_tx_pdu_ctxt0_process,
                                PDU_CTXT0_P) // Exit Slot

exit:
    // TODO - release busy if dgst computation failed and fail the command?
    add            SESSDGST_CB_P, r0, k.phv_global_common_cb_addr
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS,
                                CAPRI_TABLE_SIZE_512_BITS,
                                nvme_sesspostdgst_tx_cb_writeback_process,
                                SESSDGST_CB_P) // Exit Slot

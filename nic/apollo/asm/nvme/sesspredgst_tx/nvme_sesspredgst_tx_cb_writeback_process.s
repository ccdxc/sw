#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s4_t0_nvme_sesspredgst_tx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s4_t0_nvme_sesspredgst_tx_k_ k;
struct s4_t0_nvme_sesspredgst_tx_cb_writeback_process_d d;

#define PDU_CTXT_P   r1
#define DB_ADDR      r2
#define DB_DATA      r3

%%
    .param nvme_tx_pdu_context_base
    .param nvme_sesspredgst_tx_hdgst_process

.align
nvme_sesspredgst_tx_cb_writeback_process:
    tblmincri      SESSPREDGSTTX_C_INDEX, d.log_num_entries, 1
    tblmincri.f    d.wb_r0_busy, 1, 1

    addui          PDU_CTXT_P, r0, hiword(nvme_tx_pdu_context_base)
    addi           PDU_CTXT_P, PDU_CTXT_P, loword(nvme_tx_pdu_context_base)
    add            PDU_CTXT_P, PDU_CTXT_P, k.to_s4_info_pduid, LOG_PDU_CTXT_SIZE
    add            PDU_CTXT_P, PDU_CTXT_P, NVME_PDU_CTXT_PAGE_LIST_OFFSET

    CAPRI_SETUP_DB_ADDR(DB_ADDR_BASE, DB_SET_PINDEX, DB_SCHED_WR_EVAL_RING, \
                        K_GLOBAL_LIF, NVME_QTYPE_SESS, DB_ADDR)
    CAPRI_SETUP_DB_DATA(K_GLOBAL_QID, SESSPOSTDGST_TX_RING_ID, \
                        SESSPREDGSTTX_C_INDEX, DB_DATA)

    phvwrpair      p.hdgst_desc_doorbell_addr, DB_ADDR.dx, \
                   p.hdgst_desc_doorbell_data, DB_DATA.dx

    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS,
                                CAPRI_TABLE_SIZE_512_BITS,
                                nvme_sesspredgst_tx_hdgst_process,
                                PDU_CTXT_P) // Exit Slot

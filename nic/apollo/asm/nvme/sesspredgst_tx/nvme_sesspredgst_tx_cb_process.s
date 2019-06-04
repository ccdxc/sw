#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s0_t0_nvme_sesspredgst_tx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s0_t0_nvme_sesspredgst_tx_k_ k;
struct s0_t0_nvme_sesspredgst_tx_cb_process_d d;

#define SESS_WQE_P r1

%%
    .param    nvme_sesspredgst_tx_sess_wqe_process

.align
nvme_sesspredgst_tx_cb_process:
    // sesspredgst event gets triggered in ring0, so
    // check for r0_busy before proceeding
    seq            c1, d.r0_busy, d.wb_r0_busy
    bcf            [!c1], exit
    tblwr          d.ring_empty_sched_eval_done, 0 // BD Slot

    // take r1_busy lock and block other sesspostdgst events
    tblmincri.f    d.r0_busy, 1, 1  // table flush

    // copy global data from intrinsic
    phvwrpair      p.phv_global_common_lif, CAPRI_TXDMA_INTRINSIC_LIF, \
                   p.phv_global_common_cb_addr, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR
    phvwrpair      p.phv_global_common_qid, CAPRI_TXDMA_INTRINSIC_QID, \
                   p.phv_global_common_qtype, CAPRI_TXDMA_INTRINSIC_QTYPE

    // set sesspredgst dma_cmd ptr in phv
    phvwr          p.p4_txdma_intr_dma_cmd_ptr, NVME_SESS_PRE_DGST_TX_DMA_CMD_PTR

    sll            SESS_WQE_P, SESSPREDGSTTX_C_INDEX, LOG_SESS_Q_ENTRY_SIZE
    add            SESS_WQE_P, d.base_addr, SESS_WQE_P

    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS,
                                CAPRI_TABLE_SIZE_512_BITS,
                                nvme_sesspredgst_tx_sess_wqe_process,
                                SESS_WQE_P) // Exit Slot

exit:
    phvwr.e        p.p4_intr_global_drop, 1
    nop            // Exit Slot

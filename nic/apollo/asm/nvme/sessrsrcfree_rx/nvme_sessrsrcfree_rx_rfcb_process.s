#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s0_t0_nvme_sessrsrcfree_rx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s0_t0_nvme_sessrsrcfree_rx_k_ k;
struct s0_t0_nvme_sessrsrcfree_rx_rfcb_process_d d;

#define RF_WQE_P r1

%%
    .param  nvme_sessrsrcfree_rx_rfwqe_process
    .param  nvme_resourcecb_addr
    .param  nvme_nmdpr_resourcecb_addr
    .param  nvme_sessrsrcfree_rx_resourcecb_process
    .param  nvme_sessrsrcfree_rx_nmdpr_resourcecb_process

.align
nvme_sessrsrcfree_rx_rfcb_process:
    
    /*  
     *  sess-rsrc-free queue gets invoked for each PDU on ring0 after PDU 
     *  is finished processing  by sess-post-dgst-rx or sess-post-xts-rx rings.
     *  
     *  Work here is -
     *  1. Check for busy flag and drop phv if set. Acquire r0_busy if not set.
     *  2. Load cmd_ctxt to check and post completion if completion_done is not set in rfcb.
     *  3. if in_progress is set (non 1st pass) -
     *       a. load resourcecb if no pages to free (num_pages - cur_page = 0). 
                This is to free pduid/cmdid.
     *       b. load nmdpr_resourcecb otherwise. 
     *       This is to free all remaining pages starting cur_page.
     *  4. Load rfwqe if in_progress is not set (1st Pass).
     */


    .brbegin
    brpri   r7[MAX_SESSRSRCFREE_RX_RINGS-1:0], [RF_PRI]
    nop

    .brcase     RF_RING_ID

        seq             c1, d.r0_busy, d.wb_r0_busy
        bcf             [!c1], exit
        tblwr           d.ring_empty_sched_eval_done, 0 // BD Slot

        // take r0_busy lock and block other sessrsrcfree events
        tblmincri.f     d.r0_busy, 1, 1  // table flush

        // copy global data from intrinsic
        phvwrpair       p.phv_global_common_lif, CAPRI_TXDMA_INTRINSIC_LIF, \
                        p.phv_global_common_cb_addr, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR
        phvwrpair       p.phv_global_common_qid, CAPRI_TXDMA_INTRINSIC_QID, \
                        p.phv_global_common_qtype, CAPRI_TXDMA_INTRINSIC_QTYPE

        // if first-pass load rfwqe.
        bbeq            d.in_progress, 0, load_rfwqe
        // set sessrsrcfree dma_cmd ptr in phv
        phvwr           p.p4_txdma_intr_dma_cmd_ptr, NVME_SESS_RSRC_FREE_RX_DMA_CMD_PTR  //BD-Slot
        // if (num_pages - cur_page = 0) 
        seq             c2, d.num_pages, d.cur_page
        bcf             [c2], load_resourcecb

        // load nmdpr_resourcecb
        addui           r6, r0, hiword(nvme_nmdpr_resourcecb_addr) // BD-slot
        addi            r6, r6, loword(nvme_nmdpr_resourcecb_addr)
        
        CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS,
                                    CAPRI_TABLE_SIZE_0_BITS,
                                    nvme_sessrsrcfree_rx_nmdpr_resourcecb_process,
                                    r6)

load_rfwqe:
        sll     RF_WQE_P, SESSRSRCFREERX_C_INDEX, LOG_SESS_RF_Q_ENTRY_SIZE
        add     RF_WQE_P, d.base_addr, RF_WQE_P

        CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS,
                                    CAPRI_TABLE_SIZE_512_BITS,
                                    nvme_sessrsrcfree_rx_rfwqe_process,
                                    RF_WQE_P) // Exit Slot

load_resourcecb:
        addui   r6, r0, hiword(nvme_resourcecb_addr)
        addi    r6, r6, loword(nvme_resourcecb_addr)

        CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS,
                                    CAPRI_TABLE_SIZE_0_BITS,
                                    nvme_sessrsrcfree_rx_resourcecb_process,
                                    r6)

    .brcase     MAX_SESSRSRCFREE_RX_RINGS
        bbeq            d.ring_empty_sched_eval_done, 1, exit
        nop         //BD Slot

        // ring doorbell to re-evaluate the scheduler
        DOORBELL_NO_UPDATE(CAPRI_TXDMA_INTRINSIC_LIF,
                           CAPRI_TXDMA_INTRINSIC_QTYPE,
                           CAPRI_TXDMA_INTRINSIC_QID,
                           r2, r3)
        tblwr.e         d.ring_empty_sched_eval_done, 1
        phvwr           p.p4_intr_global_drop, 1        //Exit Slot
    .brend

exit:
    phvwr.e        p.p4_intr_global_drop, 1
    nop            // Exit Slot

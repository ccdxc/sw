#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s0_t0_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s0_t0_k_ k;
struct s0_t0_nvme_sessprexts_tx_cb_process_d d;

%%
    .param  nvme_sessprexts_tx_sess_wqe_process
    .param  nvme_sessprexts_tx_nmdpr_process

.align
nvme_sessprexts_tx_cb_process:
    seq         c1, d.r0_busy, d.wb_r0_busy
    bcf         [!c1], exit
    tblwr       d.ring_empty_sched_eval_done, 0 //BD Slot

    tblmincri.f d.r0_busy, 1, 1

    //set dma cmd ptr
    phvwr       p.p4_txdma_intr_dma_cmd_ptr, NVME_SESSPREXTSTX_DMA_CMD_PTR

    //populate global data
    phvwrpair   p.phv_global_common_lif, CAPRI_TXDMA_INTRINSIC_LIF, \
                p.phv_global_common_cb_addr, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR
    phvwrpair   p.phv_global_common_qid, CAPRI_TXDMA_INTRINSIC_QID, \
                p.phv_global_common_qtype, CAPRI_TXDMA_INTRINSIC_QTYPE

    // if needed, checkout a page
    seq         c1, d.page_ptr, r0
    bcf         [!c1], skip_page_alloc
    phvwr       p.to_s3_info_page_ptr, d.page_ptr   //BD Slot

    addi        r2, r0, NVME_TX_NMDPR_ALLOC_IDX
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS,
                              CAPRI_TABLE_SIZE_64_BITS,
                              nvme_sessprexts_tx_nmdpr_process,
                              r2)

skip_page_alloc:
    //calculate session_wqe address = base_addr + (ci << sizeof(wqe))
    sll         r1, SESSPREXTSTX_C_INDEX, LOG_SESS_Q_ENTRY_SIZE
    add         r1, d.base_addr, r1

    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS,
                                CAPRI_TABLE_SIZE_16_BITS,
                                nvme_sessprexts_tx_sess_wqe_process,
                                r1) //Exit Slot 

exit:
    phvwr.e     p.p4_intr_global_drop, 1
    nop             //Exit Slot
    

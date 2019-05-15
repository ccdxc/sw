#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s0_t0_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s0_t0_k_ k;
struct s0_t0_nvme_req_tx_sqcb_process_d d;

%%
    .param  nvme_req_tx_sqe_process

.align
nvme_req_tx_sqcb_process:
    .brbegin
    brpri       r7[MAX_REQ_TX_RINGS-1:0], [SQ_PRI]
    nop

    .brcase     SQ_RING_ID
        //XXX: change to two busy bytes, one in stage 0 and one in writeback stage
        bbeq        d.busy, 1, exit
        tblwr       d.ring_empty_sched_eval_done, 0 //BD Slot 

        //take the busy lock
        tblwr.f     d.busy, 1

        //set dma cmd ptr
        phvwr       p.p4_txdma_intr_dma_cmd_ptr, NVME_REQ_TX_DMA_CMD_PTR

        //populate global data
        phvwrpair   p.phv_global_common_lif, CAPRI_TXDMA_INTRINSIC_LIF, \
                    p.phv_global_common_cb_addr, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR
        phvwrpair   p.phv_global_common_qid, CAPRI_TXDMA_INTRINSIC_QID, \
                    p.phv_global_common_qtype, CAPRI_TXDMA_INTRINSIC_QTYPE

        //calculate the sqe address = sq_base_addr + (ci << sizeof(sqe))
        sll         r1, SQ_C_INDEX, d.log_wqe_size
        add         r1, d.sq_base_addr, r1

        phvwr       p.to_s1_info_lif_ns_start, d.lif_ns_start
        phvwr       p.to_s2_info_log_host_page_size, d.log_host_page_size

        CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS,
                                    CAPRI_TABLE_SIZE_512_BITS,
                                    nvme_req_tx_sqe_process,
                                    r1) //Exit Slot

    .brcase     MAX_REQ_TX_RINGS
        bbeq        d.ring_empty_sched_eval_done, 1, exit
        nop         //BD Slot

        // ring doorbell to re-evalue the scheduler
        DOORBELL_NO_UPDATE(CAPRI_TXDMA_INTRINSIC_LIF,
                           CAPRI_TXDMA_INTRINSIC_QTYPE,
                           CAPRI_TXDMA_INTRINSIC_QID,
                           r2, r3)
        tblwr.e     d.ring_empty_sched_eval_done, 1        
        phvwr       p.p4_intr_global_drop, 1        //Exit Slot
    .brend

exit:
    phvwr.e     p.p4_intr_global_drop, 1
    nop         //Exit Slot


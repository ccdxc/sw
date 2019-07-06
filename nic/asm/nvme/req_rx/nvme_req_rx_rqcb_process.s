#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s0_t0_nvme_req_rx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s0_t0_nvme_req_rx_k_ k;
struct s0_t0_nvme_req_rx_rqcb_process_d d;

#define TO_S2_P    to_s2_info
#define RQE_PTR    r1

%%
    .param  nvme_req_rx_rqe_process
    .param  nvme_req_rx_resourcecb_process
    .param  nvme_resourcecb_addr

.align
nvme_req_rx_rqcb_process:

    /* This is the consumer context of SERQ cb. 
       When TCP produces an element in SERQ, it rings the doorbell of rqcb’s ring 0.
       Consume the element at SERQ CI
       If busy is set, phv drop
       else set busy to 1. If curr_pg_ptr is set, load pdu_hdr, else load tcp wqe (entry at CI)
                           If pdu_offset is 0, load resource_cb
     */

    .brbegin
    brpri       r7[MAX_REQ_RX_RINGS-1:0], [RQ_PRI]
    nop

    .brcase     RQ_RING_ID

        bbne        d.r0_busy, d.wb_r0_busy, exit
        tblwr       d.ring_empty_sched_eval_done, 0 // BD Slot 

        //take the busy lock
        tblmincri.f d.r0_busy, 1, 1

        seq         c1, d.resource_alloc_done, 0
        // c1: resource not allocated
        bcf         [!c1], skip_resource_alloc
        addui       r6, r0, hiword(nvme_resourcecb_addr) // BD Slot

        addi        r6, r6, loword(nvme_resourcecb_addr)
        CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_EN,
                                  CAPRI_TABLE_SIZE_512_BITS,
                                  nvme_req_rx_resourcecb_process,
                                  r6)

skip_resource_alloc:
        // set dma cmd ptr
        phvwr       p.p4_txdma_intr_dma_cmd_ptr, NVME_REQ_RX_DMA_CMD_PTR

        // populate global data
        phvwrpair   p.phv_global_common_lif, CAPRI_TXDMA_INTRINSIC_LIF, \
                    p.phv_global_common_cb_addr, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR
        phvwrpair   p.phv_global_common_qid, CAPRI_TXDMA_INTRINSIC_QID, \
                    p.phv_global_common_qtype, CAPRI_TXDMA_INTRINSIC_QTYPE
    
        CAPRI_SET_FIELD2(TO_S2_P, session_id, d.session_id)
                    
        // calculate the rqe address = rq_base_addr + (ci << sizeof(rqe))
        sll         RQE_PTR, RQ_C_INDEX, d.log_wqe_size
        add         RQE_PTR, d.base_addr, RQE_PTR

        // TODO optimization to load pdu_hdr directly if there are multiple PDU's in a page
        CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS,
                                    CAPRI_TABLE_SIZE_512_BITS,
                                    nvme_req_rx_rqe_process,
                                    RQE_PTR) // Exit Slot

    .brcase     MAX_REQ_RX_RINGS
        bbeq        d.ring_empty_sched_eval_done, 1, exit
        nop // BD Slot

        // ring doorbell to re-evalue the scheduler
        DOORBELL_NO_UPDATE(CAPRI_TXDMA_INTRINSIC_LIF,
                           CAPRI_TXDMA_INTRINSIC_QTYPE,
                           CAPRI_TXDMA_INTRINSIC_QID,
                           r2, r3)
        tblwr.e     d.ring_empty_sched_eval_done, 1        
        phvwr       p.p4_intr_global_drop, 1 // Exit Slot
    .brend

exit:
    phvwr.e     p.p4_intr_global_drop, 1
    nop // Exit Slot

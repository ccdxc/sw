#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "common_phv.h"

struct req_tx_phv_t p;
struct sqcb0_t d;
struct smbdc_req_tx_s0_t0_k k;

#define SQCB_TO_WQE_P t0_s2s_sqcb_to_wqe_info
#define TO_RDMA_CQE_P t0_s2s_rdma_cqe_info

%%
    .param    smbdc_req_tx_wqe_process
    .param    smbdc_req_tx_rdma_cqe_process

.align
smbdc_req_tx_sqcb_process:

#   // moving to _ext program
#   // copy intrinsic to global
#   add            r1, r0, offsetof(struct phv_, common_global_global_data) 

#   // enable below code after spr_tbladdr special purpose register is available in capsim
#   #mfspr         r1, spr_tbladdr
#   CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, cb_addr, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR_WITH_SHIFT(SQCB_ADDR_SHIFT))
#   CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, lif, CAPRI_TXDMA_INTRINSIC_LIF)
#   CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, qtype, CAPRI_TXDMA_INTRINSIC_QTYPE)
#   CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, qid, CAPRI_TXDMA_INTRINSIC_QID)

#   //set dma_cmd_ptr in phv
#   TXDMA_DMA_CMD_PTR_SET(REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_START_FLIT_CMD_ID)

    .brbegin
    brpri          r7[MAX_SQ_RINGS-1: 0], [TIMER_PRI, SQ_PRI, RDMA_CQ_PROXY_PRI]
    nop

    .brcase        SQ_RING_ID

        // Check if cindex is pointing to yet to be filled wqe
        seq            c3, SQ_C_INDEX, SQ_P_INDEX
        bcf            [c3], exit
        
        // reset sched_eval_done 
        tblwr          d.ring_empty_sched_eval_done, 0

        add            r1, r0, SQ_C_INDEX
        
        sll            r2, r1, d.log_wqe_size
        add            r2, r2, d.sq_base_addr
       
        CAPRI_RESET_TABLE_0_ARG()

        CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, smbdc_req_tx_wqe_process, r2)
        
        tblmincri      SQ_C_INDEX, d.log_num_wqes, 1 

        //setup dma_cmd for wqe_context, also set end of commands
        //r1 already has SQ_C_INDEX
        sll            r2, r1, LOG_WQE_CONTEXT_SIZE
        add            r2, r2, d.sq_wqe_context_base_addr
        DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_ID_WQE_CONTEXT)
        DMA_HBM_PHV2MEM_SETUP(r6, smbdc_wqe_context, smbdc_wqe_context, r2)
        DMA_SET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, r6)

        nop.e
        nop

    .brcase        TIMER_RING_ID
        nop.e
        nop

    .brcase        RDMA_CQ_PROXY_RING_ID
        seq        c3, RDMA_CQ_PROXY_C_INDEX, RDMA_CQ_PROXY_P_INDEX
        bcf        [c3], exit

        add        r1, r0, RDMA_CQ_PROXY_C_INDEX
        sll        r2, r1, d.rdma_cq_log_wqe_size
        add        r2, r2, d.rdma_cq_base_addr

        CAPRI_RESET_TABLE_0_ARG()
        
        add            r1, r0, d.sq_unack_pindex
        sll            r3, r1, LOG_WQE_CONTEXT_SIZE
        add            r3, r3, d.sq_wqe_context_base_addr

        CAPRI_SET_FIELD2(TO_RDMA_CQE_P, wqe_context_addr, r3)

        CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, smbdc_req_tx_rdma_cqe_process, r2)

        tblmincri      RDMA_CQ_PROXY_C_INDEX, d.rdma_cq_log_num_wqes, 1

        //prepare doorbell to set RDMA CQ's cindex. r5 for db_addr and r6 for db_data
        PREPARE_DOORBELL_WRITE_CINDEX(d.rdma_cq_lif, d.rdma_cq_qtype, d.rdma_cq_qid, d.rdma_cq_ring_id, RDMA_CQ_PROXY_C_INDEX, r5, r6)
        phvwr          p.db_data, r6
        DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_ID_RDMA_CQ)
        DMA_HBM_PHV2MEM_SETUP(r6, db_data, db_data, r5)
        DMA_SET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, r6)

        nop.e
        nop

    .brcase        MAX_SQ_RINGS

        bbeq    d.ring_empty_sched_eval_done, 1, exit
        nop     //BD Slot                        
        
        // ring doorbell to re-evaluate scheduler
        DOORBELL_NO_UPDATE(CAPRI_TXDMA_INTRINSIC_LIF, CAPRI_TXDMA_INTRINSIC_QTYPE, CAPRI_TXDMA_INTRINSIC_QID, r2, r3)
        tblwr   d.ring_empty_sched_eval_done, 1
        
        phvwr   p.common.p4_intr_global_drop, 1
        nop.e
        nop

    .brend

exit:
    phvwr   p.common.p4_intr_global_drop, 1
    nop.e
    nop

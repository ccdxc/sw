#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s5_t0_nvme_req_tx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s5_t0_nvme_req_tx_k_ k;
struct s5_t0_nvme_req_tx_sessprodcb_process_d d;

#define DB_ADDR r4
#define DB_DATA r5
#define DMA_CMD_BASE r6

%%
    .param  nvme_req_tx_sqcb_writeback_process

.align
nvme_req_tx_sessprodcb_process:

    mfspr       r1, spr_mpuid
    seq         c1, r1[4:2], STAGE_5
    bcf         [!c1], bubble_to_next_stage
    nop         //BD Slot

    //XXX: check if resourcecb allocation was successful
    //XXX: check for full q (choke counter)
    //XXX: check for read/write opcode
    
    
    sll         r3, XTS_Q_PI, LOG_SESS_Q_ENTRY_SIZE
    tblmincri.f XTS_Q_PI, d.log_num_xts_q_entries, 1    //Flush
    add         r3, d.xts_q_base_addr, r3

    //sess_wqe is populated in cmdid/pduid_fetch_process, but DMA cmd is enqueued here
    DMA_CMD_BASE_GET(DMA_CMD_BASE, session_wqe_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, sess_wqe_cmdid, sess_wqe_pduid, r3)

    //ring the doorbell of xts_q
    CAPRI_SETUP_DB_ADDR(DB_ADDR_BASE, DB_SET_PINDEX, DB_SCHED_WR_EVAL_RING, \
                        K_GLOBAL_LIF, NVME_QTYPE_SESSXTSTX, DB_ADDR)
    CAPRI_SETUP_DB_DATA(d.xts_qid, SESSPREXTS_TX_RING_ID, XTS_Q_PI, DB_DATA) 

    phvwr       p.session_db_data, DB_DATA.dx
    
    DMA_CMD_BASE_GET(DMA_CMD_BASE, session_db_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, session_db_data, session_db_data, DB_ADDR)
    DMA_SET_WR_FENCE_END_OF_CMDS(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)

    add         r1, r0, k.phv_global_common_cb_addr
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_EN,
                                CAPRI_TABLE_SIZE_512_BITS,
                                nvme_req_tx_sqcb_writeback_process,
                                r1) //Exit Slot
  
bubble_to_next_stage:
    seq         c1, r1[4:2], STAGE_4
    bcf         [!c1], exit
    nop         //BD Slot
    
    nop.e
    CAPRI_NEXT_TABLE0_SET_SIZE(CAPRI_TABLE_LOCK_EN,
                               CAPRI_TABLE_SIZE_512_BITS)   //Exit Slot
    
exit:
    nop.e
    nop         //Exit Slot


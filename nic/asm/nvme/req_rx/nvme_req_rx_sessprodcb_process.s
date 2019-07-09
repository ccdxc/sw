#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s4_t0_nvme_req_rx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s4_t0_nvme_req_rx_k_ k;
struct s4_t0_nvme_req_rx_sessprodrxcb_process_d d;

#define TO_S_WB_P        to_s5_info
#define DB_ADDR          r4
#define DB_DATA          r5
#define DMA_CMD_BASE     r6

%%
    .param  nvme_req_rx_rqcb_writeback_process

.align
nvme_req_rx_sessprodcb_process:

    mfspr       r1, spr_mpuid
    seq         c1, r1[4:2], STAGE_4
    bcf         [!c1], bubble_to_next_stage

    // if dgst_Q CI = (dgst_Q PI + 1) % num_dgst_q_entries,
    // dgst q is full
    add         r1, r0, DGST_Q_PI // BD Slot
    mincr       r1, d.log_num_dgst_q_entries, 1
    seq         c2, DGST_Q_CI, r1
    bcf         [c2], sess_prod_dgst_full

    // TODO check for read/write opcode
    
    sll         r3, DGST_Q_PI, LOG_SESS_Q_ENTRY_SIZE // BD Slot
    tblmincri.f DGST_Q_PI, d.log_num_dgst_q_entries, 1    //Flush
    add         r3, d.dgst_q_base_addr, r3

    //sess_wqe is populated in cmdid/pduid_fetch_process, but DMA cmd is enqueued here
    DMA_CMD_BASE_GET(DMA_CMD_BASE, session_wqe_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, sess_wqe_cmdid, sess_wqe_pduid, r3)

    add         r1, d.dgst_qid, NVME_SESS_DGST_RX, LOG_MAX_NUM_SESSIONS
    //ring the doorbell of dgst_q
    CAPRI_SETUP_DB_ADDR(DB_ADDR_BASE, DB_SET_PINDEX, DB_SCHED_WR_EVAL_RING, \
                        K_GLOBAL_LIF, NVME_QTYPE_SESS, DB_ADDR)
    CAPRI_SETUP_DB_DATA(r1, SESSPREDGST_RX_RING_ID, DGST_Q_PI, DB_DATA)

    phvwr       p.session_db_data, DB_DATA.dx
    
    DMA_CMD_BASE_GET(DMA_CMD_BASE, session_db_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, session_db_data, session_db_data, DB_ADDR)
    DMA_SET_WR_FENCE_END_OF_CMDS(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)

load_writeback:
    add         r1, r0, k.phv_global_common_cb_addr
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_EN,
                                CAPRI_TABLE_SIZE_512_BITS,
                                nvme_req_rx_rqcb_writeback_process,
                                r1) // Exit Slot

sess_prod_dgst_full:
    b           load_writeback
    CAPRI_SET_FIELD2(TO_S_WB_P, sess_prod_dgst_full, 1) // BD Slot

bubble_to_next_stage:
    seq         c1, r1[4:2], STAGE_3
    bcf         [!c1], exit
    nop // BD Slot
    
    nop.e
    CAPRI_NEXT_TABLE0_SET_SIZE(CAPRI_TABLE_LOCK_EN,
                               CAPRI_TABLE_SIZE_512_BITS) // Exit Slot
    
exit:
    nop.e
    nop // Exit Slot

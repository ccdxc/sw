#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s4_t0_nvme_sesspostdgst_tx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s4_t0_nvme_sesspostdgst_tx_k_ k;
struct s4_t0_nvme_sesspostdgst_tx_sessprodcb_process_d d;

#define NUM_AVAIL_TCPQ_ENTRIES r1
#define NUM_TCPQ_ENTRIES       r2
#define SESSDGST_CB_P          r2
#define TMP_R                  r2
#define TCPQ_ADDR0_P           r3
#define TCPQ_ADDR1_P           r4
#define DMA_CMD0_LEN           r5
#define DMA_CMD1_LEN           r6
#define DMA_CMD_BASE           r7

%%
    .param   nvme_sesspostdgst_tx_cb_writeback_process

.align
nvme_sesspostdgst_tx_sessprodcb_process:
    // check for number of tcpq entries available to post pages
    // if there are not sufficient entries, give up the scheduler
    // turn and come back to check again. If available, post all
    // pages taking into account of whether ring entries
    // are contiguous or wrapped around.
    slt            c1, TCP_Q_PI, TCP_Q_CI
    sub            NUM_AVAIL_TCPQ_ENTRIES, TCP_Q_PI, TCP_Q_CI
    mincr.c1       NUM_AVAIL_TCPQ_ENTRIES, d.log_num_tcp_q_entries, 0
    sll            NUM_TCPQ_ENTRIES, 1, d.log_num_tcp_q_entries
    sub            NUM_AVAIL_TCPQ_ENTRIES, NUM_TCPQ_ENTRIES, NUM_AVAIL_TCPQ_ENTRIES
    // only num_q_entries -1 can be filled in a ring
    sub            NUM_AVAIL_TCPQ_ENTRIES, NUM_AVAIL_TCPQ_ENTRIES, 1

    sle            c2, k.to_s4_info_num_pages, NUM_AVAIL_TCPQ_ENTRIES
    bcf            [!c2], sessdgst_cb_writeback

    // compose DMA cmds in phv, while tso_process will
    // prepare the actual tcp_wqe to DMA based on page_ptr and length
    // in the cmd_ctxt
    add            TCPQ_ADDR0_P, d.tcp_q_base_addr, TCP_Q_PI, LOG_HBM_AL_RING_ENTRY_SIZE // BD Slot

    sub            DMA_CMD0_LEN, NUM_TCPQ_ENTRIES, TCP_Q_PI
    slt            c3, DMA_CMD0_LEN, k.to_s4_info_num_pages
    bcf            [!c3], tcp_wqe_dma0
    cmov           DMA_CMD0_LEN, c3, DMA_CMD0_LEN, k.to_s4_info_num_pages // BD Slot

tcp_wqe_dma1:
    // DMA cmds have to be posted in two parts due to wrap around
    // tcp_wqe_dma1 cmd will be remaining pages starting from p_index 0
    DMA_CMD_BASE_GET(DMA_CMD_BASE, tcp_wqe_dma1)
    sub            DMA_CMD1_LEN, k.to_s4_info_num_pages, DMA_CMD0_LEN
    add            DMA_CMD1_LEN, r0, DMA_CMD1_LEN, LOG_HBM_AL_RING_ENTRY_SIZE
    add            DMA_CMD0_LEN, r0, DMA_CMD0_LEN, LOG_HBM_AL_RING_ENTRY_SIZE
    add            TCPQ_ADDR1_P, d.tcp_q_base_addr, r0
    DMA_HBM_PHV2MEM_START_SLEN_ELEN_SETUP(DMA_CMD_BASE, TMP_R, tcp_wqe0_pad, DMA_CMD0_LEN, DMA_CMD1_LEN, TCPQ_ADDR1_P)

tcp_wqe_dma0:
    DMA_CMD_BASE_GET(DMA_CMD_BASE, tcp_wqe_dma0)
    add.!c3        DMA_CMD0_LEN, r0, DMA_CMD0_LEN, LOG_HBM_AL_RING_ENTRY_SIZE
    DMA_HBM_PHV2MEM_START_LEN_SETUP(DMA_CMD_BASE, TMP_R, tcp_wqe0_pad, DMA_CMD0_LEN, TCPQ_ADDR0_P)

    // Increment session's dgst ring ci as this dgst compuation is completed
    tblmincri      DGST_Q_CI, d.log_num_dgst_q_entries, 1
    // Increment tcpq pi_index by number of pages posted
    add            r3, k.to_s4_info_num_pages, r0 // BD Slot
    tblmincr.f     TCP_Q_PI, d.log_num_tcp_q_entries, r3

    // DMA cmd to ring tcp sesq upon enqueuing tcp pages
    DMA_CMD_BASE_GET(DMA_CMD_BASE, tcp_db_dma)
    add            r2, d.tcpcb_sesq_db_data, TCP_Q_PI
    phvwr          p.tcp_db_data, r2.dx
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, tcp_db_data, tcp_db_data, d.tcpcb_sesq_db_addr)
    DMA_SET_WR_FENCE_END_OF_CMDS(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)

sessdgst_cb_writeback:
    // TODO - just release busy if there in no available entries in tcpq
    add            SESSDGST_CB_P, r0, k.phv_global_common_cb_addr
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_EN,
                                CAPRI_TABLE_SIZE_512_BITS,
                                nvme_sesspostdgst_tx_cb_writeback_process,
                                SESSDGST_CB_P) // Exit Slot

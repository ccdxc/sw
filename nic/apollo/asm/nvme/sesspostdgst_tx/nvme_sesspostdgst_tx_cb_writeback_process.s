#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s5_t0_nvme_sesspostdgst_tx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s5_t0_nvme_sesspostdgst_tx_k_ k;
struct s5_t0_nvme_sesspostdgst_tx_cb_writeback_process_d d;

#define T0_PDU_CTXT_P   r1
#define T1_PDU_CTXT_P   r2
#define T0_NUM_PAGES    r3
#define T1_NUM_PAGES    r4
#define TCP_WQE_OFFSET  r5

%%
    .param nvme_tx_pdu_context_base
    .param nvme_resourcecb_addr
    .param nvme_sesspostdgst_tx_cb_tso_process
    .param nvme_sesspostdgst_tx_resourcecb_process

.align
nvme_sesspostdgst_tx_cb_writeback_process:
    tblmincri      SESSPOSTDGSTTX_C_INDEX, d.log_num_entries, 1
    tblmincri.f    d.wb_r1_busy, 1, 1

    // Load resourcecb to fetch pindex to free pdu_id
    addui   r1, r0, hiword(nvme_resourcecb_addr)
    addi    r1, r1, loword(nvme_resourcecb_addr)

    // Pin resourcecb process to stage 6 for freeing pduid.
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_EN,
                              CAPRI_TABLE_SIZE_512_BITS,
                              nvme_sesspostdgst_tx_resourcecb_process,
                              r1)

    addui          T0_PDU_CTXT_P, r0, hiword(nvme_tx_pdu_context_base)
    addi           T0_PDU_CTXT_P, T0_PDU_CTXT_P, loword(nvme_tx_pdu_context_base)
    add            T0_PDU_CTXT_P, T0_PDU_CTXT_P, k.to_s5_info_pduid, LOG_PDU_CTXT_SIZE
    add            T0_PDU_CTXT_P, T0_PDU_CTXT_P, NVME_PDU_CTXT_PAGE_LIST_OFFSET

    // If more than 8 pages are there load tso_process in
    // two tables, one for first 64 bytes of page ptrs and another
    // for next 64 bytes
    sle            c1, k.to_s5_info_num_pages, 8
    bcf            [c1], t0_tso_process
    cmov           T0_NUM_PAGES, c1, k.to_s5_info_num_pages, 8

t1_tso_process:
    sub            T1_NUM_PAGES, k.to_s5_info_num_pages, 8
    add            TCP_WQE_OFFSET, r0, T0_NUM_PAGES, LOG_HBM_AL_RING_ENTRY_SIZE
    phvwrpair      p.t1_s2s_writeback_to_tso_info_is_t0, 0, \
                   p.t1_s2s_writeback_to_tso_info_tcp_wqe_offset, TCP_WQE_OFFSET
    phvwr          p.t1_s2s_writeback_to_tso_info_num_pages, T1_NUM_PAGES
    add            T1_PDU_CTXT_P, T0_PDU_CTXT_P, 64
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_DIS,
                              CAPRI_TABLE_SIZE_512_BITS,
                              nvme_sesspostdgst_tx_cb_tso_process,
                              T1_PDU_CTXT_P)

t0_tso_process:
    CAPRI_RESET_TABLE_0_ARG()
    phvwr          p.t0_s2s_writeback_to_tso_info_num_pages, T0_NUM_PAGES
    phvwrpair      p.t0_s2s_writeback_to_tso_info_is_t0, 1, \
                   p.t0_s2s_writeback_to_tso_info_tcp_wqe_offset,  0
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS,
                                CAPRI_TABLE_SIZE_512_BITS,
                                nvme_sesspostdgst_tx_cb_tso_process,
                                T0_PDU_CTXT_P) // Exit Slot

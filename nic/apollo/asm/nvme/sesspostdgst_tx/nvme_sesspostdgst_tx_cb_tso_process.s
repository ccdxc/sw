#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s6_t0_nvme_sesspostdgst_tx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
// Same program is invoked  in Table 0 and Table 1 depending on the number
// of tcp wqes to be posted. Since the structure definition for s6_t0_k_
// and s6_t1_k_ are same, k[] declaration based on one structure type is
// sufficient
struct s6_t0_nvme_sesspostdgst_tx_k_ k;
struct s6_t0_nvme_sesspostdgst_tx_cb_tso_process_t0_d d;

#define PHV_TCP_WQE_BASE              r1
#define PDU_CTXT_PAGE_ENTRY_BASE      r2
#define NUM_PAGES                     r3
#define TCP_WQE_DESCR_ADDR            r4
#define TCP_PAGE_LEN                  r5
#define TCP_PAGE_P                    r6
#define DMA_CMD_BASE                  r7

%%

.align
nvme_sesspostdgst_tx_cb_tso_process:

    add            PHV_TCP_WQE_BASE, offsetof(struct phv_, tcp_wqe0_descr_addr), \
                   k.t0_s2s_writeback_to_tso_info_tcp_wqe_offset
    add            PDU_CTXT_PAGE_ENTRY_BASE, r0, \
                   offsetof(struct s6_t0_nvme_sesspostdgst_tx_cb_tso_process_t0_d, page0)
    add            NUM_PAGES, k.t0_s2s_writeback_to_tso_info_num_pages, r0

    // phvwr as many tcp_wqes as there are number of tcp pages to post. phv2mem
    // dma cmd for these tcp_wqes have been already composed in
    // sesspostdgst_tx_sessprodcb_process
write_one_tcp_wqe:
    tblrdp         TCP_PAGE_P, PDU_CTXT_PAGE_ENTRY_BASE, \
                   offsetof(struct nvme_pdu_ctxt_page_entry_t, page_addr), \
                   sizeof(struct nvme_pdu_ctxt_page_entry_t.page_addr)
    tblrdp         TCP_PAGE_LEN, PDU_CTXT_PAGE_ENTRY_BASE, \
                   offsetof(struct nvme_pdu_ctxt_page_entry_t, len), \
                   sizeof(struct nvme_pdu_ctxt_page_entry_t.len)

    // skip past scratch bytes and point to the start of pkt_Descr in the page
    add            TCP_WQE_DESCR_ADDR, TCP_PAGE_P, PKT_DESC_SCRATCH_OVERHEAD
    phvwrp         PHV_TCP_WQE_BASE, offsetof(struct hbm_al_ring_entry_t, descr_addr), \
                   sizeof(struct hbm_al_ring_entry_t.descr_addr), TCP_WQE_DESCR_ADDR
    phvwrp         PHV_TCP_WQE_BASE, offsetof(struct hbm_al_ring_entry_t, len), \
                   sizeof(struct hbm_al_ring_entry_t.len), TCP_PAGE_LEN

    sub            PDU_CTXT_PAGE_ENTRY_BASE, PDU_CTXT_PAGE_ENTRY_BASE, NVME_PDU_CTXT_PAGE_ENTRY_SIZE
    sub            NUM_PAGES, NUM_PAGES, 1
    seq            c1, NUM_PAGES, 0
    bcf            [!c1], write_one_tcp_wqe
    sub            PHV_TCP_WQE_BASE, PHV_TCP_WQE_BASE, sizeof(struct hbm_al_ring_entry_t) // BD Slot

    // compose DMA cmd for hdgst in t0 program. Depending on the total number of
    // pages posted, compose DMA cmd for ddgst either in t0 or t1 program
    seq            c1, k.t0_s2s_writeback_to_tso_info_is_t0, 1
    bcf            [!c1], setup_ddgst_dma
    sle            c2, k.to_s6_info_total_pages, 8 // BD Slot
setup_hdgst_dma:
    DMA_CMD_BASE_GET(DMA_CMD_BASE, hdgst_dma)
    add            r1, d.page0, (PKT_DESC_OVERHEAD + NVME_O_TCP_CMD_CAPSULE_CH_PSH_SIZE)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, hdgst_data, hdgst_data, r1)
    nop.!c2.e
    CAPRI_SET_TABLE_0_VALID(0)

setup_ddgst_dma:
    DMA_CMD_BASE_GET(DMA_CMD_BASE, ddgst_dma)
    sub            r1, TCP_PAGE_LEN,  NVME_O_TCP_DDGST_SIZE
    add            r1, TCP_PAGE_P, r1
    CAPRI_SET_TABLE_1_VALID_C(!c1, 0)
    DMA_HBM_PHV2MEM_SETUP_E(DMA_CMD_BASE, ddgst_data, ddgst_data, r1)

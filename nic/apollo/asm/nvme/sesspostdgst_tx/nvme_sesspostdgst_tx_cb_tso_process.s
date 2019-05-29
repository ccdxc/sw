#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s5_t0_nvme_sesspostdgst_tx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
// Same program is invoked  in Table 0 and Table 1 depending on the number
// of tcp wqes to be posted. Since the structure definition for s5_t0_k_
// and s5_t1_k_ are same, k[] declaration based on one structure type is
// sufficient
struct s5_t0_nvme_sesspostdgst_tx_k_ k;

#define PHV_TCP_WQE_BASE              r1
#define PDU_CTXT_PAGE_ENTRY_BASE  r2
#define NUM_PAGES                     r3
#define TCP_WQE_DESCR_ADDR            r4
#define TCP_WQE_LEN                   r5

%%

.align
nvme_sesspostdgst_tx_cb_tso_process:

    add            PHV_TCP_WQE_BASE, offsetof(struct phv_, tcp_wqe0_pad), \
                   k.t0_s2s_writeback_to_tso_info_tcp_wqe_offset
    add            PDU_CTXT_PAGE_ENTRY_BASE, r0, r0
    add            NUM_PAGES, k.t0_s2s_writeback_to_tso_info_num_pages, r0

    // phvwr as many tcp_wqes as there are number of tcp pages to post. phv2mem
    // dma cmd for these tcp_wqes have been already composed in
    // sesspostdgst_tx_sessprodcb_process
write_one_tcp_wqe:
    tblrdp         TCP_WQE_DESCR_ADDR, PDU_CTXT_PAGE_ENTRY_BASE, \
                   offsetof(struct nvme_pdu_ctxt_page_entry_t, page_addr), \
                   sizeof(struct nvme_pdu_ctxt_page_entry_t.page_addr)
    tblrdp         TCP_WQE_LEN, PDU_CTXT_PAGE_ENTRY_BASE, \
                   offsetof(struct nvme_pdu_ctxt_page_entry_t, len), \
                   sizeof(struct nvme_pdu_ctxt_page_entry_t.len)

    phvwrp         PHV_TCP_WQE_BASE, offsetof(struct hbm_al_ring_entry_t, descr_addr), \
                   sizeof(struct hbm_al_ring_entry_t.descr_addr), TCP_WQE_DESCR_ADDR
    phvwrp         PHV_TCP_WQE_BASE, offsetof(struct hbm_al_ring_entry_t, len), \
                   sizeof(struct hbm_al_ring_entry_t.len), TCP_WQE_LEN

    add            PDU_CTXT_PAGE_ENTRY_BASE, PDU_CTXT_PAGE_ENTRY_BASE, NVME_PDU_CTXT_PAGE_ENTRY_SIZE
    sub            NUM_PAGES, NUM_PAGES, 1
    seq            c1, NUM_PAGES, 0
    bcf            [!c1], write_one_tcp_wqe
    add            PHV_TCP_WQE_BASE, PHV_TCP_WQE_BASE, sizeof(struct hbm_al_ring_entry_t) // BD Slot

    seq            c1, k.t0_s2s_writeback_to_tso_info_is_t0, 1
    CAPRI_SET_TABLE_0_VALID_CE(c1, 0)
    CAPRI_SET_TABLE_1_VALID_CE(!c1, 0)
    nop


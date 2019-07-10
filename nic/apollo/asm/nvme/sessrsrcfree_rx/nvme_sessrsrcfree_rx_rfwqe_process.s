#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s1_t0_nvme_sessrsrcfree_rx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s1_t0_nvme_sessrsrcfree_rx_k_ k;
struct s1_t0_nvme_sessrsrcfree_rx_rfwqe_process_d d;

#define PDU_CTXT1_P             r1
#define PDU_CTXT2_P             r2
#define NUM_PAGE_PTRS_PER_D     8

%%
    .param  nvme_sessrsrcfree_rx_pdu_ctxt_fetch_pages_1_process
    .param  nvme_sessrsrcfree_rx_pdu_ctxt_fetch_pages_2_process
    .param  nvme_rx_pdu_context_base

.align
nvme_sessrsrcfree_rx_rfwqe_process:

    // calculate pdu_ctxt page-list offset address
    // nvme_rx_pdu_context_base + (pdu_id << sizeof(pdu_context)) + pdu_ctxt_page_list_offset
    addui          PDU_CTXT1_P, r0, hiword(nvme_rx_pdu_context_base)
    addi           PDU_CTXT1_P, PDU_CTXT1_P, loword(nvme_rx_pdu_context_base)
    add            PDU_CTXT1_P, PDU_CTXT1_P, d.pdu_id, LOG_PDU_CTXT_SIZE
    add            PDU_CTXT1_P, PDU_CTXT1_P, NVME_PDU_CTXT_PAGE_LIST_OFFSET 

    phvwr          p.to_s2_info_num_pages, d.num_pages
    phvwr          p.to_s3_info_num_pages, d.num_pages
    phvwrpair      p.to_s5_info_pduid, d.pdu_id, p.to_s5_info_cmdid, d.cmd_id
    phvwrpair      p.to_s6_info_pduid, d.pdu_id, p.to_s6_info_cmdid, d.cmd_id

    // fetch first-page and last-page address from pdu_ctxt page-list.
    sle            c1, d.num_pages, NUM_PAGE_PTRS_PER_D
    bcf            [c1], load_pdu_ctxt_fetch_pages_1_process
    phvwrpair      p.to_s4_info_num_pages, d.num_pages, \
                   p.to_s4_info_pdu_ctxt_page_list_offset_addr, PDU_CTXT1_P // BD-Slot


    // Load last 8 pages in table1 if num_pages > 8
    // last-8-pages-addr = PDU_CTXT1_P + ((num_pages - 1) / 8) * 64
    sub            r3, d.num_pages, 1
    div            r3, r3, NUM_PAGE_PTRS_PER_D
    sll            r3, r3, CAPRI_LOG_SIZEOF_U64_BITS
    add            PDU_CTXT2_P, PDU_CTXT1_P, r3

    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_DIS,
                              CAPRI_TABLE_SIZE_512_BITS,
                              nvme_sessrsrcfree_rx_pdu_ctxt_fetch_pages_2_process,
                              PDU_CTXT2_P) 

load_pdu_ctxt_fetch_pages_1_process:
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS,
                                CAPRI_TABLE_SIZE_512_BITS,
                                nvme_sessrsrcfree_rx_pdu_ctxt_fetch_pages_1_process,
                                PDU_CTXT1_P) // Exit Slot

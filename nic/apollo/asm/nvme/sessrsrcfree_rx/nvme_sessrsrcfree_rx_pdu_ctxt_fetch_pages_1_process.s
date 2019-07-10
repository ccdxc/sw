#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s2_t0_nvme_sessrsrcfree_rx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s2_t0_nvme_sessrsrcfree_rx_k_ k;
struct s2_t0_nvme_sessrsrcfree_rx_pdu_ctxt_fetch_pages_1_process_d d;

#define FIRST_PAGE_ADDR_P             r1
#define LAST_PAGE_ADDR_P              r2
#define NUM_PAGE_PTRS_PER_D           8

%%
    .param  nvme_sessrsrcfree_rx_decr_refcnt_first_page_process
    .param  nvme_sessrsrcfree_rx_decr_refcnt_last_page_process

.align
nvme_sessrsrcfree_rx_pdu_ctxt_fetch_pages_1_process:

    // If num-pages is >8 last_page will be loaded by pdu_ctxt_fetch_pages_2_process
    // If num-pages is 1, then load only first page
    sle     c1, k.to_s2_info_num_pages, NUM_PAGE_PTRS_PER_D 
    seq     c2, k.to_s2_info_num_pages, 1

    bcf     [!c1 | c2], load_first_page_process 
    add     FIRST_PAGE_ADDR_P, r0, d.page_addr1 //BD-Slot

    // load last page
    // last_page_addr_p = (u64 *)(d_p + (sizeof(u64) * (num_pages - 1)))
    sub     r3, k.to_s2_info_num_pages, 1
    sll     r3, r3, CAPRI_LOG_SIZEOF_U64_BITS
    tblrdp  LAST_PAGE_ADDR_P, r3, 0, CAPRI_SIZEOF_U64_BITS 
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_DIS,
                              CAPRI_TABLE_SIZE_512_BITS,
                              nvme_sessrsrcfree_rx_decr_refcnt_last_page_process,
                              LAST_PAGE_ADDR_P) 

load_first_page_process:
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS,
                                CAPRI_TABLE_SIZE_512_BITS,
                                nvme_sessrsrcfree_rx_decr_refcnt_first_page_process,
                                FIRST_PAGE_ADDR_P) // Exit Slot

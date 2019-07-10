#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s2_t1_nvme_sessrsrcfree_rx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s2_t1_nvme_sessrsrcfree_rx_k_ k;
struct s2_t1_nvme_sessrsrcfree_rx_pdu_ctxt_fetch_pages_2_process_d d;

#define LAST_PAGE_ADDR_P              r1
#define NUM_PAGE_PTRS_PER_D           8 

%%
    .param  nvme_sessrsrcfree_rx_decr_refcnt_last_page_process

.align
nvme_sessrsrcfree_rx_pdu_ctxt_fetch_pages_2_process:

    // We will get here only if num_pages is > 8
    // last_page_addr_p = (u64 *)(d_p + (sizeof(u64) * ((num_pages - 1) % 8)))
    sub     r2, k.to_s2_info_num_pages, 1
    mod     r2, r2, NUM_PAGE_PTRS_PER_D
    sll     r2, r2, CAPRI_LOG_SIZEOF_U64_BITS
    tblrdp  LAST_PAGE_ADDR_P, r2, 0, CAPRI_SIZEOF_U64_BITS 
    CAPRI_NEXT_TABLE1_READ_PC_E(CAPRI_TABLE_LOCK_EN,
                               CAPRI_TABLE_SIZE_512_BITS,
                               nvme_sessrsrcfree_rx_decr_refcnt_last_page_process,
                               LAST_PAGE_ADDR_P) 

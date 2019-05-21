#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s1_t2_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s1_t2_k_ k;
struct s1_t2_nvme_sessprexts_tx_nmdpr_process_d d;

#define NMDPR_ENTRY_P   r7

%%
    .param      nvme_tx_nmdpr_ring_base
    .param      nvme_sessprexts_tx_page_fetch_process

.align
nvme_sessprexts_tx_nmdpr_process:
    bbeq    d.full, 1, nmdpr_empty
    add     r1, r0, NMDPR_IDX  //BD Slot
    
    andi    r1, r1, NMDPR_RING_SIZE_MASK

    phvwr   p.to_s2_info_incr_num_pages, 1
    
    addui   NMDPR_ENTRY_P, r0, hiword(nvme_tx_nmdpr_ring_base)
    addi    NMDPR_ENTRY_P, NMDPR_ENTRY_P, loword(nvme_tx_nmdpr_ring_base)
    add     NMDPR_ENTRY_P, NMDPR_ENTRY_P, r1, LOG_NMDPR_RING_ENTRY_SIZE

    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS,
                                CAPRI_TABLE_SIZE_512_BITS,
                                nvme_sessprexts_tx_page_fetch_process,
                                NMDPR_ENTRY_P) //Exit Slot

nmdpr_empty:
    //XXX: TBD
    nop.e
    CAPRI_SET_TABLE_2_VALID(0)  //Exit Slot

#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s4_t1_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s4_t1_k_ k;
struct s4_t1_nvme_req_tx_resourcecb_process_d d;

#define CMDID_RING_CI_OFFSET \
    FIELD_OFFSET(s4_t1_nvme_req_tx_resourcecb_process_d, cmdid_ring_ci)

#define PDUID_RING_CI_OFFSET \
    FIELD_OFFSET(s4_t1_nvme_req_tx_resourcecb_process_d, pduid_ring_ci)

#define DMA_CMD_BASE    r5

%%
    .param  nvme_cmd_context_ring_base
    .param  nvme_tx_pdu_context_ring_base
    .param  nvme_req_tx_cmdid_fetch_process
    .param  nvme_req_tx_pduid_fetch_process

.align
nvme_req_tx_resourcecb_process:
    mfspr       r1, spr_mpuid
    seq         c1, r1[4:2], STAGE_4
    bcf         [!c1], bubble_to_next_stage
    nop         //BD Slot

    //XXX: check for ring empty

    //checkout a new pduid
    add             r2, r0, PDUID_RING_PROXY_CI
    tblmincri       PDUID_RING_PROXY_CI, d.pduid_ring_log_sz, 1

    //XXX: check for ring empty

    //checkout a new cmdid
    add             r1, r0, CMDID_RING_PROXY_CI
    tblmincri.f     CMDID_RING_PROXY_CI, d.cmdid_ring_log_sz, 1 //Flush

    addui           r6, r0, hiword(nvme_cmd_context_ring_base)
    addi            r6, r6, loword(nvme_cmd_context_ring_base)
    add             r6, r6, r1, LOG_CMDID_RING_ENTRY_SIZE
     
    phvwr           p.cmdid_cindex_index, CMDID_RING_PROXY_CI_LE
    mfspr           r3, spr_tbladdr 
    add             r3, r3, CMDID_RING_CI_OFFSET

    DMA_CMD_BASE_GET(DMA_CMD_BASE, cmdid_cindex_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, cmdid_cindex_index, cmdid_cindex_index, r3)
    
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_DIS,
                              CAPRI_TABLE_SIZE_16_BITS,
                              nvme_req_tx_cmdid_fetch_process,
                              r6)

    addui           r6, r0, hiword(nvme_tx_pdu_context_ring_base)
    addi            r6, r6, loword(nvme_tx_pdu_context_ring_base)
    add             r6, r6, r2, LOG_PDUID_RING_ENTRY_SIZE
     
    phvwr           p.pduid_cindex_index, PDUID_RING_PROXY_CI_LE
    mfspr           r3, spr_tbladdr 
    add             r3, r3, PDUID_RING_CI_OFFSET

    DMA_CMD_BASE_GET(DMA_CMD_BASE, pduid_cindex_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, pduid_cindex_index, pduid_cindex_index, r3)
    
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS,
                                CAPRI_TABLE_SIZE_16_BITS,
                                nvme_req_tx_pduid_fetch_process,
                                r6) //Exit Slot

bubble_to_next_stage:
    seq         c1, r1[4:2], STAGE_3
    bcf         [!c1], exit
    nop         //BD Slot
    
    nop.e
    CAPRI_NEXT_TABLE1_SET_SIZE(CAPRI_TABLE_LOCK_EN,
                               CAPRI_TABLE_SIZE_512_BITS)   //Exit Slot
    
exit:
    nop.e
    nop         //Exit Slot


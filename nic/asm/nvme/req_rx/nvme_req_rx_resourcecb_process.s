#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s1_t1_nvme_req_rx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s1_t1_nvme_req_rx_k_ k;
struct s1_t1_nvme_req_rx_resourcecb_process_d d;

#define TO_S2_PDU_P     to_s2_info
#define RX_PDUID_RING_CI_OFFSET \
    FIELD_OFFSET(s1_t1_nvme_req_rx_resourcecb_process_d, rx_pduid_ring_ci)

#define DMA_CMD_BASE    r5

%%

    .param  nvme_rx_pdu_context_ring_base
    .param  nvme_req_rx_pduid_fetch_process

.align
nvme_req_rx_resourcecb_process:

    // check for ring empty
    seq             c1, RX_PDUID_RING_PI, RX_PDUID_RING_PROXY_CI
    bcf             [c1], resource_cb_empty

    //checkout a new pduid
    add             r2, r0, RX_PDUID_RING_PROXY_CI // BD Slot
    tblmincri.f     RX_PDUID_RING_PROXY_CI, d.rx_pduid_ring_log_sz, 1

    addui           r6, r0, hiword(nvme_rx_pdu_context_ring_base)
    addi            r6, r6, loword(nvme_rx_pdu_context_ring_base)
    add             r6, r6, r2, RX_LOG_PDUID_RING_ENTRY_SIZE

    phvwr           p.pduid_cindex_index, RX_PDUID_RING_PROXY_CI
    mfspr           r3, spr_tbladdr
    add             r3, r3, RX_PDUID_RING_CI_OFFSET

    DMA_CMD_BASE_GET(DMA_CMD_BASE, pduid_cindex_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, pduid_cindex_index, pduid_cindex_index, r3)

    CAPRI_NEXT_TABLE1_READ_PC_E(CAPRI_TABLE_LOCK_DIS,
                                CAPRI_TABLE_SIZE_16_BITS,
                                nvme_req_rx_pduid_fetch_process,
                                r6) // Exit Slot

resource_cb_empty:
    CAPRI_SET_FIELD2(TO_S2_PDU_P, resourcecb_empty, 1)
    CAPRI_SET_TABLE_1_VALID(0) // Exit Slot

exit:
    nop.e
    nop // Exit Slot

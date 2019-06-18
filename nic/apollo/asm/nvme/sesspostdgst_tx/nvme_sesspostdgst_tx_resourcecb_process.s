#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s6_t2_nvme_sesspostdgst_tx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s6_t2_nvme_sesspostdgst_tx_k_ k;
struct s6_t2_nvme_sesspostdgst_tx_resourcecb_process_d d;

#define TX_PDUID_RING_PI_OFFSET \
    FIELD_OFFSET(s6_t2_nvme_sesspostdgst_tx_resourcecb_process_d, tx_pduid_ring_pi)

#define DMA_CMD_BASE   r7
%%

    .param     nvme_tx_pdu_context_ring_base

.align
nvme_sesspostdgst_tx_resourcecb_process:
    // ring full should not happen, but check for it and assert
    add            r1, r0, TX_PDUID_RING_PROXY_PI
    tblmincri.f    TX_PDUID_RING_PROXY_PI, d.tx_pduid_ring_log_sz, 1

    addui          r2, r0, hiword(nvme_tx_pdu_context_ring_base)
    addi           r2, r2, loword(nvme_tx_pdu_context_ring_base)
    add            r2, r2, r1, TX_LOG_PDUID_RING_ENTRY_SIZE

    // write pduid into the pduid_ring_entry
    phvwr          p.pduid_index, k.to_s6_info_pduid
    DMA_CMD_BASE_GET(DMA_CMD_BASE, pduid_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, pduid_index, pduid_index, r2)

    // write updated pindex into the resource cb
    phvwr          p.pduid_pindex_index, TX_PDUID_RING_PROXY_PI
    mfspr          r1, spr_tbladdr
    add            r1, r1, TX_PDUID_RING_PI_OFFSET
    DMA_CMD_BASE_GET(DMA_CMD_BASE, pduid_pindex_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, pduid_pindex_index, pduid_pindex_index, r1)
    // Fence pindex write into resource cb on write to pduid_ring_entry to
    // make sure pindex is updated only after the free pduid is written to the ring
    DMA_SET_WR_FENCE_E(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)

    CAPRI_SET_TABLE_2_VALID(0) // Exit Slot

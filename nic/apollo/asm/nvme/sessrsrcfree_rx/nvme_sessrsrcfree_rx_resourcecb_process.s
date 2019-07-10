#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s5_t0_nvme_sessrsrcfree_rx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s5_t0_nvme_sessrsrcfree_rx_k_ k;
struct s5_t0_nvme_sessrsrcfree_rx_resourcecb_process_d d;

#define RX_PDUID_RING_PI_OFFSET \
    FIELD_OFFSET(s5_t0_nvme_sessrsrcfree_rx_resourcecb_process_d, rx_pduid_ring_pi)

#define CMDID_RING_PI_OFFSET \
    FIELD_OFFSET(s5_t0_nvme_sessrsrcfree_rx_resourcecb_process_d, cmdid_ring_pi)

#define DMA_CMD_BASE   r7

%%
    .param  nvme_sessrsrcfree_rx_rfcb_writeback_process
    .param  nvme_rx_pdu_context_ring_base
    .param  nvme_cmd_context_ring_base

.align
nvme_sessrsrcfree_rx_resourcecb_process:

    /*
     * Free PDU-ID
     */

    // TODO: ring full should not happen, but check for it and assert
    add            r1, r0, RX_PDUID_RING_PROXY_PI
    tblmincri.f    RX_PDUID_RING_PROXY_PI, d.rx_pduid_ring_log_sz, 1

    addui          r2, r0, hiword(nvme_rx_pdu_context_ring_base)
    addi           r2, r2, loword(nvme_rx_pdu_context_ring_base)
    add            r2, r2, r1, RX_LOG_PDUID_RING_ENTRY_SIZE

    // write pduid into the pduid_ring_entry
    phvwr          p.pduid_index, k.to_s5_info_pduid
    DMA_CMD_BASE_GET(DMA_CMD_BASE, free_pduid_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, pduid_index, pduid_index, r2)

    // write updated pindex into the resource cb
    phvwr          p.pduid_pindex_index, RX_PDUID_RING_PROXY_PI
    mfspr          r1, spr_tbladdr
    add            r1, r1, RX_PDUID_RING_PI_OFFSET
    DMA_CMD_BASE_GET(DMA_CMD_BASE, pduid_pindex_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, pduid_pindex_index, pduid_pindex_index, r1)
    /*
     * Fence pindex write into resource cb on write to pduid_ring_entry to
     * make sure pindex is updated only after the free pduid is written to the ring
     */
    DMA_SET_WR_FENCE(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)

    /*
     * Free CMD-ID
     */
    // TODO: For Read check if command can be freed.
    // TODO: ring full should not happen, but check for it and assert
    add            r1, r0, CMDID_RING_PROXY_PI
    tblmincri.f    CMDID_RING_PROXY_PI, d.cmdid_ring_log_sz, 1

    addui          r2, r0, hiword(nvme_cmd_context_ring_base)
    addi           r2, r2, loword(nvme_cmd_context_ring_base)
    add            r2, r2, r1, LOG_CMDID_RING_ENTRY_SIZE

    // write cmdid into the cmdid_ring_entry
    phvwr          p.cmdid_index, k.to_s5_info_cmdid
    DMA_CMD_BASE_GET(DMA_CMD_BASE, free_cmdid_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, cmdid_index, cmdid_index, r2)

    // write updated pindex into the resource cb
    phvwr          p.cmdid_pindex_index, CMDID_RING_PROXY_PI
    mfspr          r1, spr_tbladdr
    add            r1, r1, CMDID_RING_PI_OFFSET
    DMA_CMD_BASE_GET(DMA_CMD_BASE, cmdid_pindex_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, cmdid_pindex_index, cmdid_pindex_index, r1)
    /*
     * Fence pindex write into resource cb on write to cmdid_ring_entry to
     * make sure pindex is updated only after the free cmdid is written to the ring
     */
    // TODO: For write, EOC will always happen here. For Read, it may happen in pduid-free.
    DMA_SET_WR_FENCE_END_OF_CMDS(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)

    add     r1, r0, k.phv_global_common_cb_addr
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_EN,
                                CAPRI_TABLE_SIZE_512_BITS,
                                nvme_sessrsrcfree_rx_rfcb_writeback_process,
                                r1)

#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s3_t0_nvme_sesspredgst_tx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s3_t0_nvme_sesspredgst_tx_k_ k;
struct s3_t0_nvme_sesspredgst_tx_dgstcb_process_d d;

#define SESSDGST_CB_P   r1
#define DMA_CMD_BASE    r5

%%
    .param    nvme_sesspredgst_tx_cb_writeback_process

.align
nvme_sesspredgst_tx_dgstcb_process:
    // For every PDU 2 digests have to be computed, one for hdr and another
    // for data. So look for 2 slots in dgst_ring and if not, unset busy
    // and try in the next scheduler turn
    add            r1, r0, d.pi
    mincr          r1, d.log_sz, 1
    seq            c1, r1, d.ci
    mincr          r1, d.log_sz, 1
    seq            c2, r1, d.ci
    bcf            [c1 | c2], sessdgst_cb_writeback
    add            r2, r0, d.pi
    tblwr.f        d.pi, r1

    // r2 maintains PI index for data dgst desc slot

    // DMA cmd for data dgst desc
    // opaque_tag_Address is already set to dgst_cb's CI offset
    // in pdu_ctxt_process where dgst_cb base address is computed
    //phvwrpair      p.ddgst_desc_opaque_tag_value, r2 \
    //               p.ddgst_desc_opaque_tag_write_en, 1
    sll            r3, r2, LOG_DGST_DESC_SIZE
    add            r3, d.dgst_ring_base_addr, r3
    DMA_CMD_BASE_GET(DMA_CMD_BASE, ddgst_desc_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, ddgst_desc_src, ddgst_desc_status_data, r3);

    // Increment r2 - PI index to hdr dgst desc slot
    mincr          r2, d.log_sz, 1

    // Setup opaque_tag_data with updated PI, which will be programmed as CI
    // after completion of hdr and data dgst computation by the engine
    phvwr          p.hdgst_desc_opaque_tag_data, r1

    // DMA cmd for hdr dgst desc
    sll            r3, r2, LOG_DGST_DESC_SIZE
    add            r3, d.dgst_ring_base_addr, r3
    DMA_CMD_BASE_GET(DMA_CMD_BASE, hdgst_desc_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, hdgst_desc_src, hdgst_desc_status_data, r3);

    //DMA cmd for dgst doorbell
    phvwr          p.dgst_db_index, r1.wx
    DMA_CMD_BASE_GET(DMA_CMD_BASE, dgst_db_dma)
    addi        r1, r0, HW_DGST_DB_ADDR
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, dgst_db_index, dgst_db_index, r1)
    DMA_SET_WR_FENCE_END_OF_CMDS(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)

sessdgst_cb_writeback:
    // TODO free busy lock and wait for next scheduler
    // opportunity
    add            SESSDGST_CB_P, r0, k.phv_global_common_cb_addr
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_EN,
                                CAPRI_TABLE_SIZE_512_BITS,
                                nvme_sesspredgst_tx_cb_writeback_process,
                                SESSDGST_CB_P) // Exit Slot


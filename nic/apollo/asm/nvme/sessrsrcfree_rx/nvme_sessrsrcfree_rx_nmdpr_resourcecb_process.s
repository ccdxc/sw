#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s4_t0_nvme_sessrsrcfree_rx_k.h"
#include "capri.h"
#include "nvme_common.h"
#include "capri-macros.h"


struct phv_ p;
struct s4_t0_nvme_sessrsrcfree_rx_k_ k;
struct s4_t0_nvme_sessrsrcfree_rx_nmdpr_resourcecb_process_d d;

#define RX_NMDPR_RING_PI_OFFSET \
    FIELD_OFFSET(s4_t0_nvme_sessrsrcfree_rx_nmdpr_resourcecb_process_d, rx_nmdpr_ring_pi)

#define NUM_PAGES_TO_FREE   r1
#define DMA_CMD_BASE        r7

%%
    .param  nvme_resourcecb_addr
    .param  nvme_sessrsrcfree_rx_resourcecb_process
    .param  RNMDPR_BIG_TABLE_BASE

.align
nvme_sessrsrcfree_rx_nmdpr_resourcecb_process:

    // Calculate num_pages_to_free
    crestore    [c2,c1], k.{to_s4_info_free_first_page, to_s4_info_free_last_page} , 0x3
    seq         c3, k.to_s4_info_num_pages, 1
    setcf       c4, [!c3 & !c1]

    add         NUM_PAGES_TO_FREE, r0, k.to_s4_info_num_pages

    // Remove first-page from to-free list if free_first_page is NOT set.
    sub.!c2     NUM_PAGES_TO_FREE, NUM_PAGES_TO_FREE, 1

    // Remove last-page from to-free list if free_last_page is NOT set and num_pages is >1
    sub.c4      NUM_PAGES_TO_FREE, NUM_PAGES_TO_FREE, 1

    // Skip this stage if no pages to free.
    beq         NUM_PAGES_TO_FREE, r0, load_resourcecb

free_pages:
    // TODO: ring full should not happen, but check for it and assert
    add            r2, r0, RX_NMDPR_RING_PROXY_PI // BD-slot

    tblmincr.f     RX_NMDPR_RING_PROXY_PI, d.rx_nmdpr_ring_log_sz, NUM_PAGES_TO_FREE

    // r3 = start address in rx-nmdpr
    addui          r3, r0, hiword(RNMDPR_BIG_TABLE_BASE)
    addi           r3, r3, loword(RNMDPR_BIG_TABLE_BASE)
    add            r3, r3, r2, RNMDPR_BIG_TABLE_ENTRY_SIZE_SHFT

    // r4 = distance of rx-nmdpr_pi from end of ring
    sub            r4, ASIC_RNMDPR_BIG_RING_SIZE, r2
    slt            c1, r4, NUM_PAGES_TO_FREE
    b.c1           dma_rxnmdpr_two_dma_commands

dma_rxnmdpr_one_dma_command:
    add             r5, k.to_s4_info_pdu_ctxt_page_list_offset_addr, r0  // BD-slot

    // r6 = total len in bytes
    add             r6, r0, NUM_PAGES_TO_FREE, RNMDPR_BIG_TABLE_ENTRY_SIZE_SHFT
    // r5 = Updated src_addr in pdu-ctxt based on if first-page has to be freed.
    add.!c2         r5, r5, 1, RNMDPR_BIG_TABLE_ENTRY_SIZE_SHFT
    DMA_CMD_BASE_GET(DMA_CMD_BASE, free_pages_src_dma1)
    DMA_HBM_MEM2MEM_SRC_SETUP(DMA_CMD_BASE, r6, r5)

    DMA_CMD_BASE_GET(DMA_CMD_BASE, free_pages_dst_dma1)
    DMA_HBM_MEM2MEM_DST_SETUP(DMA_CMD_BASE, r6, r3)

    b               dma_rxnmdpr_pindex

dma_rxnmdpr_two_dma_commands:
    // r6 = total len in bytes
    add             r6, r0, r4, RNMDPR_BIG_TABLE_ENTRY_SIZE_SHFT  // BD-Slot
    // r5 = Updated src_addr in pdu-ctxt based on if first-page has to be freed.
    add.!c2         r5, r5, 1, RNMDPR_BIG_TABLE_ENTRY_SIZE_SHFT

    DMA_CMD_BASE_GET(DMA_CMD_BASE, free_pages_src_dma1)
    DMA_HBM_MEM2MEM_SRC_SETUP(DMA_CMD_BASE, r6, r5)
    DMA_CMD_BASE_GET(DMA_CMD_BASE, free_pages_dst_dma1)
    DMA_HBM_MEM2MEM_DST_SETUP(DMA_CMD_BASE, r6, r3)

    // r5 = Updated src-addr for remaining pages to DMA
    add             r5, r5, r6
    // r4 is remaining entries
    sub             r4, NUM_PAGES_TO_FREE, r4
    add             r6, r0, r4, RNMDPR_BIG_TABLE_ENTRY_SIZE_SHFT

    // r3 = start address in rnmdpr
    addui           r3, r0, hiword(RNMDPR_BIG_TABLE_BASE)
    addi            r3, r3, loword(RNMDPR_BIG_TABLE_BASE)

    DMA_CMD_BASE_GET(DMA_CMD_BASE, free_pages_src_dma2)
    DMA_HBM_MEM2MEM_SRC_SETUP(DMA_CMD_BASE, r6, r5)
    DMA_CMD_BASE_GET(DMA_CMD_BASE, free_pages_dst_dma2)
    DMA_HBM_MEM2MEM_DST_SETUP(DMA_CMD_BASE, r6, r3)


dma_rxnmdpr_pindex:
    // write updated pindex into the nmdpr-resource cb
    phvwr           p.rx_nmdpr_pindex_index, RX_NMDPR_RING_PROXY_PI
    mfspr           r1, spr_tbladdr
    add             r1, r1, RX_NMDPR_RING_PI_OFFSET
    DMA_CMD_BASE_GET(DMA_CMD_BASE, rx_nmdpr_pindex_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, rx_nmdpr_pindex_index, rx_nmdpr_pindex_index, r1)
    /*
     * Fence pindex write into nmdpr resource cb on mem2mem write to nmdpr-ring  to
     * make sure pindex is updated only after the free pages is written to the ring
     */
    DMA_SET_WR_FENCE(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)

load_resourcecb:
    // load resourcecb
    addui           r6, r0, hiword(nvme_resourcecb_addr)
    addi            r6, r6, loword(nvme_resourcecb_addr)

    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_EN,
                                CAPRI_TABLE_SIZE_512_BITS,
                                nvme_sessrsrcfree_rx_resourcecb_process,
                                r6)

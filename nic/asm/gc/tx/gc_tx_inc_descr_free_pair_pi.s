#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"

struct phv_ p;
struct gc_tx_read_descr_free_pair_pi_read_descr_free_pair_pi_d d;
struct gc_tx_read_descr_free_pair_pi_k k;

#define Common_phv_desc_addr {common_phv_desc_addr_sbit0_ebit31...common_phv_desc_addr_sbit32_ebit33}

%%
    .param          RNMDPR_BIG_TABLE_BASE
    .param          TNMDPR_BIG_TABLE_BASE

.align
gc_tx_inc_rnmdr_free_pair_pi:
    CAPRI_OPERAND_DEBUG(d.index)
    CAPRI_CLEAR_TABLE_VALID(0)
    /*
     * Read page FP.PI, to get index to write the freed descr address to
     */
dma_cmd_rnmdr:
    /*
     * Write descriptor address to descr[FP.PI]
     */
    phvwri          p.p4_txdma_intr_dma_cmd_ptr, CAPRI_PHV_START_OFFSET(ringentry1_dma_dma_cmd_type) / 16

    addui           r3, r0, hiword(RNMDPR_BIG_TABLE_BASE)
    addi            r3, r3, loword(RNMDPR_BIG_TABLE_BASE)
    and             r2, d.{index}.wx, ((1 << CAPRI_RNMDPR_BIG_RING_SHIFT) - 1)
    phvwr           p.t0_s2s_idx, r2
    add             r3, r3, r2, RNMDPR_BIG_TABLE_ENTRY_SIZE_SHFT
    phvwr           p.ring_entry1_descr_addr, k.Common_phv_desc_addr
    CAPRI_DMA_CMD_PHV2MEM_SETUP(ringentry1_dma_dma_cmd, r3, ring_entry1_descr_addr, ring_entry1_descr_addr)
dma_rnmdr_alloc_pair_ci:
    /*
     * Set AP.CI = FP.PI
     */
    add             r2, d.{index}.wx, 1
    phvwr           p.ci_1_index, r2.wx
    addi            r3, r0, CAPRI_SEM_RNMDPR_BIG_ALLOC_CI_RAW_ADDR
    CAPRI_DMA_CMD_PHV2MEM_SETUP(ci_1_dma_dma_cmd, r3, ci_1_index, ci_1_index)
    CAPRI_DMA_CMD_STOP(ci_1_dma_dma_cmd)
    nop.e
    nop



.align
gc_tx_inc_tnmdr_free_pair_pi:
    CAPRI_OPERAND_DEBUG(d.index)
    CAPRI_CLEAR_TABLE_VALID(0)
    /*
     * Read page FP.PI, to get index to write the freed descr address to
     */
dma_cmd_tnmdr:
    /*
     * Write descriptor address to descr[FP.PI]
     */
    phvwri          p.p4_txdma_intr_dma_cmd_ptr, CAPRI_PHV_START_OFFSET(ringentry1_dma_dma_cmd_type) / 16

    addui           r3, r0, hiword(TNMDPR_BIG_TABLE_BASE)
    addi            r3, r3, loword(TNMDPR_BIG_TABLE_BASE)
    and             r2, d.{index}.wx, ((1 << CAPRI_TNMDPR_BIG_RING_SHIFT) - 1)
    phvwr           p.t0_s2s_idx, r2
    add             r3, r3, r2, TNMDPR_BIG_TABLE_ENTRY_SIZE_SHFT
    phvwr           p.ring_entry1_descr_addr, k.Common_phv_desc_addr
    CAPRI_DMA_CMD_PHV2MEM_SETUP(ringentry1_dma_dma_cmd, r3, ring_entry1_descr_addr, ring_entry1_descr_addr)
dma_tnmdr_alloc_pair_ci:
    /*
     * Set AP.CI = FP.PI
     */
    add             r2, d.{index}.wx, 1
    phvwr           p.ci_1_index, r2.wx
    addi            r3, r0, CAPRI_SEM_TNMDPR_BIG_ALLOC_CI_RAW_ADDR
    CAPRI_DMA_CMD_PHV2MEM_SETUP(ci_1_dma_dma_cmd, r3, ci_1_index, ci_1_index)
    CAPRI_DMA_CMD_STOP(ci_1_dma_dma_cmd)
    nop.e
    nop

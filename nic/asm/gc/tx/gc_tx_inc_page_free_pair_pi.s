#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"

struct phv_ p;
struct gc_tx_read_page_free_pair_pi_read_page_free_pair_pi_d d;


%%
    .param          RNMPR_TABLE_BASE
    .param          TNMPR_TABLE_BASE

.align
gc_tx_inc_rnmpr_free_pair_pi:
    CAPRI_OPERAND_DEBUG(d.index)
    CAPRI_CLEAR_TABLE_VALID(0)
dma_cmd_rnmpr:
    /*
     * Write page address to page[FP.PI]
     */
    addui           r3, r0, hiword(RNMPR_TABLE_BASE)
    addi            r3, r3, loword(RNMPR_TABLE_BASE)
    and             r2, d.{index}.wx, ((1 << CAPRI_RNMPR_RING_SHIFT) - 1)
    add             r3, r3, r2, RNMPR_TABLE_ENTRY_SIZE_SHFT
    CAPRI_DMA_CMD_PHV2MEM_SETUP(ringentry2_dma_dma_cmd, r3, t0_s2s_a0, t0_s2s_a0)
dma_rnmpr_alloc_pair_ci:
    /*
     * Set AP.CI = FP.PI
     */
    add             r2, d.{index}.wx, 1
    phvwr           p.ci_2_index, r2
    addi            r3, r0, CAPRI_SEM_RNMPR_ALLOC_CI_RAW_ADDR
    CAPRI_DMA_CMD_PHV2MEM_SETUP(ci_2_dma_dma_cmd, r3, ci_2_index, ci_2_index)
    CAPRI_DMA_CMD_STOP(ci_2_dma_dma_cmd)
    nop.e
    nop

.align
gc_tx_inc_tnmpr_free_pair_pi:
    CAPRI_OPERAND_DEBUG(d.index)
    CAPRI_CLEAR_TABLE_VALID(0)
dma_cmd_tnmpr:
    /*
     * Write page address to page[FP.PI]
     */
    addui           r3, r0, hiword(TNMPR_TABLE_BASE)
    addi            r3, r3, loword(TNMPR_TABLE_BASE)
    and             r2, d.{index}.wx, ((1 << CAPRI_TNMPR_RING_SHIFT) - 1)
    add             r3, r3, r2, TNMPR_TABLE_ENTRY_SIZE_SHFT
    CAPRI_DMA_CMD_PHV2MEM_SETUP(ringentry2_dma_dma_cmd, r3, t0_s2s_a0, t0_s2s_a0)
dma_tnmpr_alloc_pair_ci:
    /*
     * Set AP.CI = FP.PI
     */
    add             r2, d.{index}.wx, 1
    phvwr           p.ci_2_index, r2
    addi            r3, r0, CAPRI_SEM_TNMPR_ALLOC_CI_RAW_ADDR
    CAPRI_DMA_CMD_PHV2MEM_SETUP(ci_2_dma_dma_cmd, r3, ci_2_index, ci_2_index)
    CAPRI_DMA_CMD_STOP(ci_2_dma_dma_cmd)
    nop.e
    nop

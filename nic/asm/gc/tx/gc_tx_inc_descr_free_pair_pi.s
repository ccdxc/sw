#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"

struct phv_ p;
struct gc_tx_read_descr_free_pair_pi_read_descr_free_pair_pi_d d;
struct gc_tx_read_descr_free_pair_pi_k k;

#define Common_phv_desc_addr {common_phv_desc_addr_sbit0_ebit31...common_phv_desc_addr_sbit32_ebit33}

%%
    .param          RNMDR_TABLE_BASE
    .param          TNMDR_TABLE_BASE
    .param          gc_tx_inc_rnmpr_free_pair_pi
    .param          gc_tx_inc_tnmpr_free_pair_pi

.align
gc_tx_inc_rnmdr_free_pair_pi:
    CAPRI_OPERAND_DEBUG(d.index)
    /*
     * Read page FP.PI, to get index to write the freed descr address to
     */
    CAPRI_NEXT_TABLE_READ_i(0, TABLE_LOCK_DIS, gc_tx_inc_rnmpr_free_pair_pi,
                    RNMPR_FREE_IDX, TABLE_SIZE_64_BITS)
dma_cmd_rnmdr:
    /*
     * Write descriptor address to descr[FP.PI]
     */
    phvwri          p.p4_txdma_intr_dma_cmd_ptr, CAPRI_PHV_START_OFFSET(ringentry1_dma_dma_cmd_type) / 16

    addui           r3, r0, hiword(RNMDR_TABLE_BASE)
    addi            r3, r3, loword(RNMDR_TABLE_BASE)
    and             r2, d.{index}.wx, ((1 << CAPRI_RNMDR_RING_SHIFT) - 1)
    add             r3, r3, r2, RNMDR_TABLE_ENTRY_SIZE_SHFT
    phvwr           p.ring_entry1_descr_addr, k.Common_phv_desc_addr
    CAPRI_DMA_CMD_PHV2MEM_SETUP(ringentry1_dma_dma_cmd, r3, ring_entry1_descr_addr, ring_entry1_descr_addr)
dma_rnmdr_alloc_pair_ci:
    /*
     * Set AP.CI = FP.PI
     */
    add             r2, d.{index}.wx, 1
    phvwr           p.ci_1_index, r2.wx
    addi            r3, r0, CAPRI_SEM_RNMDR_ALLOC_CI_RAW_ADDR
    CAPRI_DMA_CMD_PHV2MEM_SETUP(ci_1_dma_dma_cmd, r3, ci_1_index, ci_1_index)
    nop.e
    nop



.align
gc_tx_inc_tnmdr_free_pair_pi:
    CAPRI_OPERAND_DEBUG(d.index)
    /*
     * Read page FP.PI, to get index to write the freed descr address to
     */
    CAPRI_NEXT_TABLE_READ_i(0, TABLE_LOCK_DIS, gc_tx_inc_tnmpr_free_pair_pi,
                    TNMPR_FREE_IDX, TABLE_SIZE_64_BITS)
dma_cmd_tnmdr:
    /*
     * Write descriptor address to descr[FP.PI]
     */
    phvwri          p.p4_txdma_intr_dma_cmd_ptr, CAPRI_PHV_START_OFFSET(ringentry1_dma_dma_cmd_type) / 16

    addui           r3, r0, hiword(TNMDR_TABLE_BASE)
    addi            r3, r3, loword(TNMDR_TABLE_BASE)
    and             r2, d.{index}.wx, ((1 << CAPRI_TNMDR_RING_SHIFT) - 1)
    add             r3, r3, r2, TNMDR_TABLE_ENTRY_SIZE_SHFT
    phvwr           p.ring_entry1_descr_addr, k.Common_phv_desc_addr
    CAPRI_DMA_CMD_PHV2MEM_SETUP(ringentry1_dma_dma_cmd, r3, ring_entry1_descr_addr, ring_entry1_descr_addr)
dma_tnmdr_alloc_pair_ci:
    /*
     * Set AP.CI = FP.PI
     */
    add             r2, d.{index}.wx, 1
    phvwr           p.ci_1_index, r2.wx
    addi            r3, r0, CAPRI_SEM_TNMDR_ALLOC_CI_RAW_ADDR
    CAPRI_DMA_CMD_PHV2MEM_SETUP(ci_1_dma_dma_cmd, r3, ci_1_index, ci_1_index)
    nop.e
    nop

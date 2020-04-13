#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "INGRESS_gc_tx_read_descr_free_pair_pi_k.h"

struct phv_ p;
struct gc_tx_read_descr_free_pair_pi_read_descr_free_pair_pi_d d;
struct gc_tx_read_descr_free_pair_pi_k_ k;

%%
    .param          RNMDPR_BIG_TABLE_BASE
    .param          TNMDPR_BIG_TABLE_BASE

.align
gc_tx_inc_rnmdr_free_pair_pi:
    CAPRI_OPERAND_DEBUG(d.rnmdpr_fp_pi)

    /*
     * Read page FP.PI, to get index to write the freed descr address to
     */
    and             r2, d.rnmdpr_fp_pi, ASIC_RNMDPR_BIG_RING_MASK
    tbladd.f        d.rnmdpr_fp_pi, k.common_phv_num_entries_freed

dma_cmd_rnmdr:
    /*
     * Write descriptor address to descr[FP.PI]
     */
    phvwri          p.p4_txdma_intr_dma_cmd_ptr, CAPRI_PHV_START_OFFSET(ringentry1_dma_dma_cmd_type) / 16

    // r3 = start address in rnmdpr
    addui           r3, r0, hiword(RNMDPR_BIG_TABLE_BASE)
    addi            r3, r3, loword(RNMDPR_BIG_TABLE_BASE)
    add             r3, r3, r2, RNMDPR_BIG_TABLE_ENTRY_SIZE_SHFT

    // r4 = distance of rnmdpr_pi from end of ring
    sub             r4, ASIC_RNMDPR_BIG_RING_SIZE, r2
    slt             c1, r4, k.common_phv_num_entries_freed
    b.c1            dma_rnmdr_two_dma_commands

dma_rnmdr_one_dma_command:
    // r6 = total len in bytes
    add             r6, r0, k.common_phv_num_entries_freed, 3
    // uses r7
    CAPRI_DMA_CMD_PHV2MEM_SETUP_WITH_LEN(ringentry1_dma_dma_cmd, r3, ring_entry1_descr_addr, r6)
dma_rnmdr_alloc_pair_ci:
    /*
     * Set AP.CI = FP.PI
     */
    add             r2, d.rnmdpr_fp_pi, ASIC_RNMDPR_BIG_RING_SIZE
    phvwr           p.ci_1_index, r2.wx
    addi            r3, r0, ASIC_SEM_RNMDPR_BIG_ALLOC_CI_RAW_ADDR
    CAPRI_DMA_CMD_PHV2MEM_SETUP(ci_1_dma_dma_cmd, r3, ci_1_index, ci_1_index)
    CAPRI_DMA_CMD_STOP_FENCE(ci_1_dma_dma_cmd)
    phvwri.e        p.app_header_table0_valid, 0;
    nop

dma_rnmdr_two_dma_commands:
    /*
     * Setup DMA command 1
     */
    CAPRI_DMA_CMD_PHV2MEM_SETUP_NO_OFFSETS(ringentry1_dma_dma_cmd, r3)
    phvwr           p.ringentry1_dma_dma_cmd_phv_start_addr, \
                        CAPRI_PHV_START_OFFSET(ring_entry1_descr_addr)
    add             r6, CAPRI_PHV_START_OFFSET(ring_entry1_descr_addr), r4, 3
    sub             r7, r6, 1
    phvwr           p.ringentry1_dma_dma_cmd_phv_end_addr, r7
    phvwr           p.ringentry2_dma_dma_cmd_phv_start_addr, r6

    // r4 is remaining entries
    sub             r4, k.common_phv_num_entries_freed, r4
    add             r7, r7, r4, 3
    phvwr           p.ringentry2_dma_dma_cmd_phv_end_addr, r7

    // r3 = start address in rnmdpr
    addui           r3, r0, hiword(RNMDPR_BIG_TABLE_BASE)
    addi            r3, r3, loword(RNMDPR_BIG_TABLE_BASE)

    /*
     * Setup DMA command 2
     */
    CAPRI_DMA_CMD_PHV2MEM_SETUP_NO_OFFSETS(ringentry2_dma_dma_cmd, r3)
    b dma_rnmdr_alloc_pair_ci
    nop


.align
gc_tx_inc_tnmdr_free_pair_pi:
    CAPRI_OPERAND_DEBUG(d.tnmdpr_fp_pi)

    /*
     * Read page FP.PI, to get index to write the freed descr address to
     */
    and             r2, d.tnmdpr_fp_pi, ((1 << ASIC_TNMDPR_BIG_RING_SHIFT) - 1)
    tbladd.f        d.tnmdpr_fp_pi, k.common_phv_num_entries_freed

dma_cmd_tnmdr:
    /*
     * Write descriptor address to descr[FP.PI]
     */
    phvwri          p.p4_txdma_intr_dma_cmd_ptr, CAPRI_PHV_START_OFFSET(ringentry1_dma_dma_cmd_type) / 16

    // r3 = start address in rnmdpr
    addui           r3, r0, hiword(TNMDPR_BIG_TABLE_BASE)
    addi            r3, r3, loword(TNMDPR_BIG_TABLE_BASE)
    add             r3, r3, r2, TNMDPR_BIG_TABLE_ENTRY_SIZE_SHFT

    // r4 = distance of rnmdpr_pi from end of ring
    sub             r4, ASIC_TNMDPR_BIG_RING_SIZE, r2
    slt             c1, r4, k.common_phv_num_entries_freed
    b.c1            dma_tnmdr_two_dma_commands

dma_tnmdr_one_dma_command:
    // r6 = total len in bytes
    add             r6, r0, k.common_phv_num_entries_freed, 3
    // uses r7
    CAPRI_DMA_CMD_PHV2MEM_SETUP_WITH_LEN(ringentry1_dma_dma_cmd, r3, ring_entry1_descr_addr, r6)
dma_tnmdr_alloc_pair_ci:
    /*
     * Set AP.CI = FP.PI
     */
    add             r2, d.tnmdpr_fp_pi, ASIC_TNMDPR_BIG_RING_SIZE
    phvwr           p.ci_1_index, r2.wx
    addi            r3, r0, ASIC_SEM_TNMDPR_BIG_ALLOC_CI_RAW_ADDR
    CAPRI_DMA_CMD_PHV2MEM_SETUP(ci_1_dma_dma_cmd, r3, ci_1_index, ci_1_index)
    CAPRI_DMA_CMD_STOP_FENCE(ci_1_dma_dma_cmd)
    phvwri.e        p.app_header_table0_valid, 0;
    nop

dma_tnmdr_two_dma_commands:
    /*
     * Setup DMA command 1
     */
    CAPRI_DMA_CMD_PHV2MEM_SETUP_NO_OFFSETS(ringentry1_dma_dma_cmd, r3)
    phvwr           p.ringentry1_dma_dma_cmd_phv_start_addr, \
                        CAPRI_PHV_START_OFFSET(ring_entry1_descr_addr)
    add             r6, CAPRI_PHV_START_OFFSET(ring_entry1_descr_addr), r4, 3
    sub             r7, r6, 1
    phvwr           p.ringentry1_dma_dma_cmd_phv_end_addr, r7
    phvwr           p.ringentry2_dma_dma_cmd_phv_start_addr, r6

    // r4 is remaining entries
    sub             r4, k.common_phv_num_entries_freed, r4
    add             r7, r7, r4, 3
    phvwr           p.ringentry2_dma_dma_cmd_phv_end_addr, r7

    // r3 = start address in tnmdpr
    addui           r3, r0, hiword(TNMDPR_BIG_TABLE_BASE)
    addi            r3, r3, loword(TNMDPR_BIG_TABLE_BASE)

    /*
     * Setup DMA command 2
     */
    CAPRI_DMA_CMD_PHV2MEM_SETUP_NO_OFFSETS(ringentry2_dma_dma_cmd, r3)
    b dma_tnmdr_alloc_pair_ci
    nop

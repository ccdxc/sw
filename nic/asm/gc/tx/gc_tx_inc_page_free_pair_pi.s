#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_gc_tx_read_page_free_pair_pi_k.h"
#include "capri-macros.h"

struct phv_ p;
struct gc_tx_read_page_free_pair_pi_read_page_free_pair_pi_d d;
struct gc_tx_read_page_free_pair_pi_k_ k;

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
    add             r3, r3, k.t0_s2s_idx, RNMPR_TABLE_ENTRY_SIZE_SHFT
    phvwr           p.ring_entry2_descr_addr, k.t0_s2s_a0
    CAPRI_DMA_CMD_PHV2MEM_SETUP(ringentry2_dma_dma_cmd, r3, ring_entry2_descr_addr, ring_entry2_descr_addr)
    CAPRI_DMA_CMD_STOP(ringentry2_dma_dma_cmd)
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
    add             r3, r3, k.t0_s2s_idx, TNMPR_TABLE_ENTRY_SIZE_SHFT
    phvwr           p.ring_entry2_descr_addr, k.t0_s2s_a0
    CAPRI_DMA_CMD_PHV2MEM_SETUP(ringentry2_dma_dma_cmd, r3, ring_entry2_descr_addr, ring_entry2_descr_addr)
    CAPRI_DMA_CMD_STOP(ringentry2_dma_dma_cmd)
    nop.e
    nop
    

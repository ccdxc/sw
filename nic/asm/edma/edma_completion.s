
#include "INGRESS_p.h"
#include "INGRESS_tx_table_s2_t0_k.h"

#include "defines.h"


struct phv_ p;
struct tx_table_s2_t0_k_ k;


#define   _r_intr_addr  r1        // Interrupt Assert Address
#define   _r_ptr        r5        // Current DMA byte offset in PHV
#define   _r_index      r6        // Current DMA command index in PHV


%%

.align
edma_completion:

    // Load DMA command pointer
    add             _r_index, r0, k.edma_global_dma_cur_index

    // Do we need to generate an interrupt
    seq             c1, k.edma_global_intr_enable, 1

    // DMA Completion descriptor
    DMA_CMD_PTR(_r_ptr, _r_index, r7)
    DMA_PHV2MEM_WF(_r_ptr, !c1, r0, k.edma_t0_s2s_cq_desc_addr, CAPRI_PHV_START_OFFSET(edma_comp_desc_status), CAPRI_PHV_END_OFFSET(edma_comp_desc_rsvd2), r7)
    DMA_CMD_NEXT(_r_index)

    bcf             [!c1], edma_completion_done
    nop

edma_completion_interrupt:

    addi            _r_intr_addr, r0, INTR_ASSERT_BASE
    add             _r_intr_addr, _r_intr_addr, k.edma_t0_s2s_intr_assert_index, LG2_INTR_ASSERT_STRIDE

    // DMA Interrupt
    DMA_CMD_PTR(_r_ptr, _r_index, r7)
    DMA_HBM_PHV2MEM_WF(_r_ptr, _C_TRUE, _r_intr_addr, CAPRI_PHV_START_OFFSET(edma_t0_s2s_intr_assert_data), CAPRI_PHV_END_OFFSET(edma_t0_s2s_intr_assert_data), r7)
    DMA_CMD_NEXT(_r_index)

edma_completion_done:

    // End of pipeline - Make sure no more tables will be launched
    phvwri.e.f      p.{app_header_table0_valid...app_header_table3_valid}, 0
    nop

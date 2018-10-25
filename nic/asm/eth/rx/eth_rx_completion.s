
#include "INGRESS_p.h"
#include "INGRESS_rx_table_s4_t0_k.h"

#include "defines.h"

#define __ASSEMBLY__

#include "cap_top_csr_defines.h"
#include "cap_intr_c_hdr.h"


struct phv_ p;
struct rx_table_s4_t0_k_ k;


#define   _r_intr_addr  r1        // Interrupt Assert Address
#define   _r_stats      r2        // Stats
#define   _r_ptr        r5        // Current DMA byte offset in PHV
#define   _r_index      r6        // Current DMA command index in PHV


#define INTR_BASE               CAP_ADDR_BASE_INTR_INTR_OFFSET
#define INTR_ASSERT_OFFSET      CAP_INTR_CSR_DHS_INTR_ASSERT_BYTE_OFFSET
#define INTR_ASSERT_BASE        (INTR_BASE + INTR_ASSERT_OFFSET)
#define INTR_ASSERT_STRIDE      0x4
#define LG2_INTR_ASSERT_STRIDE  0x2

%%

.align
eth_rx_completion:
    LOAD_STATS(_r_stats)

    // Load DMA command pointer
    add             _r_index, r0, k.eth_rx_global_dma_cur_index

    // Do we need to generate an interrupt
    seq             c1, k.eth_rx_global_intr_enable, 1

eth_rx_completion_entry:
    SET_STAT(_r_stats, _C_TRUE, cqe)

    // DMA Completion descriptor
    DMA_CMD_PTR(_r_ptr, _r_index, r7)
    DMA_PHV2MEM(_r_ptr, !c1, k.eth_rx_global_host_queue, k.eth_rx_t0_s2s_cq_desc_addr, CAPRI_PHV_START_OFFSET(eth_rx_cq_desc_status), CAPRI_PHV_END_OFFSET(eth_rx_cq_desc_csum_tcp_ok), r7)
    DMA_CMD_NEXT(_r_index)

    bcf             [!c1], eth_rx_completion_done
    nop

eth_rx_interrupt:
    SET_STAT(_r_stats, _C_TRUE, intr)

    addi            _r_intr_addr, r0, INTR_ASSERT_BASE
    add             _r_intr_addr, _r_intr_addr, k.eth_rx_t0_s2s_intr_assert_index, LG2_INTR_ASSERT_STRIDE

    // DMA Interrupt
    DMA_CMD_PTR(_r_ptr, _r_index, r7)
    DMA_HBM_PHV2MEM_WF(_r_ptr, c0, _r_intr_addr, CAPRI_PHV_START_OFFSET(eth_rx_t0_s2s_intr_assert_data), CAPRI_PHV_END_OFFSET(eth_rx_t0_s2s_intr_assert_data), r7)
    DMA_CMD_NEXT(_r_index)

eth_rx_completion_done:
    SAVE_STATS(_r_stats)

    // End of pipeline - Make sure no more tables will be launched
    phvwri.e.f      p.{app_header_table0_valid...app_header_table3_valid}, 0
    nop

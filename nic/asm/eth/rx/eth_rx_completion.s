
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_rx_table_s7_t0_k.h"

#include "defines.h"

struct phv_ p;
struct rx_table_s7_t0_k_ k;


#define   _r_intr_addr  r1        // Interrupt Assert Address
#define   _r_stats      r2        // Stats
#define   _r_ptr        r5        // Current DMA byte offset in PHV
#define   _r_index      r6        // Current DMA command index in PHV

#define   _c_drop       c1
#define   _c_eq         c2
#define   _c_intr       c3

%%

.align
eth_rx_completion:
    LOAD_STATS(_r_stats)

    seq             _c_drop, k.eth_rx_global_drop, 1
    seq             _c_eq, k.eth_rx_global_do_eq, 1
    seq             _c_intr, k.eth_rx_global_do_intr, 1

    // Load DMA command pointer
    add             _r_index, r0, k.eth_rx_global_dma_cur_index

eth_rx_completion_entry:
    SET_STAT(_r_stats, _C_TRUE, cqe)

    // Completion entry is last if no eq or intr
    setcf           c7, ![ _c_eq | _c_intr ]

    // DMA Completion descriptor
    DMA_CMD_PTR(_r_ptr, _r_index, r7)
    DMA_CMD_RESET(_r_ptr, _c_drop)
    DMA_PHV2MEM(_r_ptr, c7, k.eth_rx_global_host_queue, k.eth_rx_t0_s2s_cq_desc_addr, CAPRI_PHV_START_OFFSET(cq_desc_status), CAPRI_PHV_END_OFFSET(cq_desc_pkt_type), r7)
    DMA_CMD_NEXT(_r_index)

eth_rx_completion_event:
    b.!_c_eq        eth_rx_completion_interrupt
    SET_STAT(_r_stats, _c_eq, eqe)

    // Event entry is last if no intr

    // DMA Completion
    DMA_CMD_PTR(_r_ptr, _r_index, r7)
    DMA_PHV2MEM(_r_ptr, !_c_intr, k.eth_rx_global_host_queue, k.eth_rx_t0_s2s_eq_desc_addr, CAPRI_PHV_START_OFFSET(eq_desc_code), CAPRI_PHV_END_OFFSET(eq_desc_gen_color), r7)
    DMA_CMD_NEXT(_r_index)

eth_rx_completion_interrupt:
    b.!_c_intr      eth_rx_completion_done
    SET_STAT(_r_stats, _c_intr, intr)

    addi            _r_intr_addr, r0, INTR_ASSERT_BASE
    add             _r_intr_addr, _r_intr_addr, k.eth_rx_t0_s2s_intr_index, LG2_INTR_ASSERT_STRIDE
    phvwri          p.eq_desc_intr_data, 0x01000000

    // DMA Interrupt
    DMA_CMD_PTR(_r_ptr, _r_index, r7)
    DMA_CMD_RESET(_r_ptr, c2)
    DMA_HBM_PHV2MEM_WF(_r_ptr, c0, _r_intr_addr, CAPRI_PHV_START_OFFSET(eq_desc_intr_data), CAPRI_PHV_END_OFFSET(eq_desc_intr_data), r7)
    DMA_CMD_NEXT(_r_index)

eth_rx_completion_done:
    SAVE_STATS(_r_stats)

    phvwri.e.f      p.app_header_table0_valid, 0
    nop

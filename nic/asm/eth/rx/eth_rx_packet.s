
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_rx_table_s3_t0_k.h"

#include "defines.h"

struct phv_ p;
struct rx_table_s3_t0_k_ k;
struct rx_table_s3_t0_eth_rx_packet_d d;

#define   _r_pktlen     r1        // Packet length
#define   _r_addr       r2        // Buffer address
#define   _r_len        r3        // Buffer length        
#define   _r_ptr        r4        // Current DMA byte offset in PHV
#define   _r_flit       r5        // Current DMA flit offset in PHV
#define   _r_index      r6        // Current DMA command offset in PHV flit

%%

.align
eth_rx_packet:

#if 0
    DEBUG_DESCR()
#endif

    bcf         [c2 | c3 | c7], eth_rx_packet_error
    nop

    // Setup DMA CMD PTR
    phvwr       p.p4_rxdma_intr_dma_cmd_ptr, ETH_DMA_CMD_START_OFFSET

    // Load DMA command pointer
    addi        _r_flit, r0, ETH_DMA_CMD_START_FLIT
    addi        _r_index, r0, ETH_DMA_CMD_START_INDEX

    // End of pipeline - Make sure no more tables are launched
    phvwri      p.{app_header_table0_valid...app_header_table3_valid}, 0

    // Packet is larger than Buffer
    add         _r_pktlen, r0, k.eth_rx_t0_s2s_packet_len
    add         _r_len, r0, d.{len}.hx
    // TODO: We have already claimed a completion entry. Should we return it
    // or create an error completion?
    blt         _r_len, _r_pktlen, eth_rx_packet_error
    nop

    // DMA packet
    DMA_CMD_PTR(_r_ptr, _r_flit, _r_index, r7)
    DMA_PKT(_r_ptr, _r_addr, k.eth_rx_t0_s2s_packet_len)
    DMA_CMD_NEXT(_r_flit, _r_index, c7)

    // Do we need to generate an interrupt?
    seq         c1, r0, k.eth_rx_t0_s2s_intr_assert_addr

    // DMA Completion descriptor
    DMA_CMD_PTR(_r_ptr, _r_flit, _r_index, r7)
    DMA_PHV2MEM(_r_ptr, c1, k.eth_rx_global_host_queue, k.eth_rx_t0_s2s_cq_desc_addr, CAPRI_PHV_START_OFFSET(eth_rx_cq_desc_status), CAPRI_PHV_END_OFFSET(eth_rx_cq_desc_csum_tcp_ok), r7)
    DMA_CMD_NEXT(_r_flit, _r_index, c7)

    nop.e.c1
    nop

eth_rx_packet_interrupt:

    // DMA Interrupt
    DMA_CMD_PTR(_r_ptr, _r_flit, _r_index, r7)
    DMA_HBM_PHV2MEM_WF(_r_ptr, c0, k.eth_rx_t0_s2s_intr_assert_addr, CAPRI_PHV_START_OFFSET(eth_rx_t0_s2s_intr_assert_data), CAPRI_PHV_END_OFFSET(eth_rx_t0_s2s_intr_assert_data), r7)
    DMA_CMD_NEXT(_r_flit, _r_index, c7)

    nop.e
    nop

eth_rx_packet_error:
    phvwri.e    p.p4_intr_global_drop, 1
    phvwri.f    p.{app_header_table0_valid...app_header_table3_valid}, 0
 

#include "INGRESS_p.h"
#include "ingress.h"
#include "defines.h"

struct phv_ p;
struct rx_table_s1_t0_k k;
struct rx_table_s1_t0_eth_rx_packet_d d;

%%

.align
eth_rx_packet:

    // Setup DMA CMD PTR
    phvwri      p.p4_rxdma_intr_dma_cmd_ptr, RX_DMA_CMD_PTR

    // End of pipeline - Make sure no more tables are launched
    phvwri      p.app_header_table0_valid, 0
    phvwri      p.app_header_table1_valid, 0
    phvwri      p.app_header_table2_valid, 0
    phvwri      p.app_header_table3_valid, 0

    // Packet is larger than Buffer
    add         r1, r0, k.eth_rx_global_packet_len
    add         r2, r0, d.len
    blt         r2, r1, eth_rx_drop_oversize_pkt
    nop

    // DMA packet to Host Memory
    phvwri      p.dma_cmd0_dma_cmd_type, CAPRI_DMA_COMMAND_PKT_TO_MEM       // Copy from Host Memory to Packet Buffer
    phvwri      p.dma_cmd0_dma_cmd_eop, 1                                   // This is the last DMA command
    phvwri      p.dma_cmd0_dma_cmd_host_addr, 1                             // The source address is a Host address
    phvwri      p.dma_cmd0_dma_cmd_cache, 0                                 // AXI Uncached Access
#if 1
    add         r7, r0, d.{addr}.dx
    add         r7, r0, d.{len}.dx
    add         r7, r0, k.eth_rx_global_packet_len
#endif
    phvwr       p.dma_cmd0_dma_cmd_addr, d.{addr}.dx                        // Source address
    phvwr       p.dma_cmd0_dma_cmd_size, r1                                 // DMA size
    tblwr.f     d.rsvd0, r1.hx
    phvwri      p.dma_cmd0_dma_cmd_use_override_lif, 0                      // Do not override LIF id for AXI address computation
    phvwri      p.dma_cmd0_dma_cmd_override_lif, 0

    nop.e
    nop

.align
eth_rx_drop_oversize_pkt:
  phvwr           p.p4_intr_global_drop, 1
  nop.e
  nop

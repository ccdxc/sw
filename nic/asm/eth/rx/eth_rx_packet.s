
#include "INGRESS_p.h"
#include "ingress.h"
#include "defines.h"

struct phv_ p;
struct rx_table_s3_t0_k k;
struct rx_table_s3_t0_eth_rx_packet_d d;

%%

.align
eth_rx_packet:

    // Setup DMA CMD PTR
    phvwri      p.p4_rxdma_intr_dma_cmd_ptr, ETH_DMA_CMD_PTR

    // End of pipeline - Make sure no more tables are launched
    phvwri      p.{app_header_table0_valid...app_header_table3_valid}, 0

    // Packet is larger than Buffer
    add         r1, r0, k.eth_rx_t0_s2s_packet_len
    add         r2, r0, d.{len}.hx
    // TODO: We have already claimed a completion entry. Should we return it
    // or create an error completion?
    blt         r2, r1, eth_rx_drop_oversize_pkt
    nop

    // DMA packet
    phvwri      p.dma_cmd0_dma_cmd_type, CAPRI_DMA_COMMAND_PKT_TO_MEM
    phvwri      p.dma_cmd0_dma_cmd_host_addr, 1
    phvwr       p.dma_cmd0_dma_cmd_addr, d.{addr}.dx
    phvwr       p.dma_cmd0_dma_cmd_size, k.eth_rx_t0_s2s_packet_len

    // DMA CQ descriptor
    phvwri      p.dma_cmd1_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri      p.dma_cmd1_dma_cmd_eop, 1
    phvwri      p.dma_cmd1_dma_cmd_host_addr, 1
    phvwr       p.dma_cmd1_dma_cmd_addr, k.eth_rx_t0_s2s_cq_desc_addr
    phvwri.e    p.dma_cmd1_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(eth_rx_cq_desc_completion_index)
    phvwri      p.dma_cmd1_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(eth_rx_cq_desc_color)

.align
eth_rx_drop_oversize_pkt:
  phvwr.e       p.p4_intr_global_drop, 1
  nop

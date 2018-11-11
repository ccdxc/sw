#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct rx_table_s4_t1_k k;
struct rx_table_s4_t1_ipsec_rxdma_stats_update_d d;
struct phv_ p;

%%
        .align
esp_ipv4_tunnel_n2h_rxdma_ipsec_update_rx_stats:
    tbladd d.n2h_rx_pkts, 1
    add r1, k.ipsec_global_packet_length, 4 
    tbladd.f d.n2h_rx_bytes, r1 
    phvwri p.app_header_table2_valid, 0
    phvwri p.p4_rxdma_intr_dma_cmd_ptr, N2H_RXDMA_IPSEC_DMA_COMMANDS_OFFSET
    phvwri p.{dma_cmd_phv2mem_ipsec_int_dma_cmd_phv_end_addr...dma_cmd_phv2mem_ipsec_int_dma_cmd_type}, ((IPSEC_N2H_INT_END_OFFSET << 18) | (IPSEC_N2H_INT_START_OFFSET << 8) | IPSEC_PHV2MEM_CACHE_ENABLE | CAPRI_DMA_COMMAND_PHV_TO_MEM)
    phvwri p.{dma_cmd_in_desc_aol_dma_cmd_phv_end_addr...dma_cmd_in_desc_aol_dma_cmd_type}, ((IPSEC_IN_DESC_AOL_END << 18) | (IPSEC_IN_DESC_AOL_START << 8) | IPSEC_PHV2MEM_CACHE_ENABLE | CAPRI_DMA_COMMAND_PHV_TO_MEM)
    phvwri.e p.{dma_cmd_out_desc_aol_dma_cmd_phv_end_addr...dma_cmd_out_desc_aol_dma_cmd_type}, ((IPSEC_OUT_DESC_AOL_END << 18) | (IPSEC_OUT_DESC_AOL_START << 8) | IPSEC_PHV2MEM_CACHE_ENABLE | CAPRI_DMA_COMMAND_PHV_TO_MEM)
    nop 

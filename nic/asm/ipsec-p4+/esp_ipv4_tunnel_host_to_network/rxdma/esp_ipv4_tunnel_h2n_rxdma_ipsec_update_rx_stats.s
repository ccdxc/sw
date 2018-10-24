#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct rx_table_s4_t1_k k;
struct rx_table_s4_t1_ipsec_rxdma_stats_update_d d;
struct phv_ p;

%%
        .align
esp_ipv4_tunnel_h2n_rxdma_ipsec_update_rx_stats:
    phvwri p.dma_cmd_iv_salt_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri p.dma_cmd_phv2mem_ipsec_int_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri p.dma_cmd_phv2mem_ipsec_int_dma_cmd_phv_start_addr, IPSEC_INT_START_OFFSET
    phvwri p.dma_cmd_phv2mem_ipsec_int_dma_cmd_phv_end_addr, IPSEC_INT_END_OFFSET
    phvwri p.dma_cmd_fill_esp_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri p.dma_cmd_in_desc_aol_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM

    tbladd d.h2n_rx_pkts, 1
    add r1, r0, k.t1_s2s_payload_size 
    tbladd d.h2n_rx_bytes, r1 
    phvwri.e p.app_header_table2_valid, 0
    nop 

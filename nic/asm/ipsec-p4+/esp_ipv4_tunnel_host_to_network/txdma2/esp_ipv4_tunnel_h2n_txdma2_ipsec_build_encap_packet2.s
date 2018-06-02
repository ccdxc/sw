#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct tx_table_s4_t1_k k;
struct tx_table_s4_t1_tx_table_s4_t1_cfg_action_d d;
struct phv_ p;

%%
        .param IPSEC_IP_HDR_BASE
        .align
esp_ipv4_tunnel_h2n_txdma2_ipsec_build_encap_packet2:
    // Outer-IP  
    phvwri p.ip_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    // P4PLUS_TO_P4_FLAGS_UPDATE_UDP_LEN | P4PLUS_TO_P4_FLAGS_UPDATE_IP_LEN | P4PLUS_TO_P4_FLAGS_COMP_OUTER_CSUM 
    //ESP Header with IV 
    phvwri p.esp_iv_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    // Payload
    phvwri p.enc_pay_load_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    //ICV
    phvwri p.icv_header_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT

    phvwri p.eth_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwri  p.eth_hdr_dma_cmd_size, ETH_FIXED_HDR_SIZE
    phvwri p.p4_txdma_intr_dma_cmd_ptr, H2N_TXDMA2_DMA_COMMANDS_OFFSET

    phvwri.f p.{icv_header_dma_pkt_eop...icv_header_dma_cmd_eop}, 3
    nop.e
    nop 

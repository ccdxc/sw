#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct tx_table_s4_t1_k k;
struct tx_table_s4_t1_tx_table_s4_t1_cfg_action_d d;
struct phv_ p;

%%
        .align
esp_ipv4_tunnel_h2n_txdma2_ipsec_build_encap_packet2:

    // Outer-IP  
    // P4PLUS_TO_P4_FLAGS_UPDATE_UDP_LEN | P4PLUS_TO_P4_FLAGS_UPDATE_IP_LEN | P4PLUS_TO_P4_FLAGS_COMP_OUTER_CSUM 
    //ESP Header with IV 
    // Payload
    phvwri p.enc_pay_load_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwri p.enc_pay_load_dma_cmd_cache, 1
    //ICV
    phvwri p.icv_header_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwri p.icv_header_dma_cmd_cache, 1
    phvwri p.eth_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwri p.eth_hdr_dma_cmd_cache, 1
    seq c1, k.ipsec_to_stage4_is_vlan_encap, 1
    phvwri.c1  p.eth_hdr_dma_cmd_size, ETH_FIXED_HDR_SIZE
    phvwri.!c1  p.eth_hdr_dma_cmd_size, ETH_FIXED_HDR_SIZE_NO_VLAN
    phvwri.e p.icv_header_dma_cmd_pkt_eop, 1
    nop 

#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct tx_table_s4_t2_k k;
struct tx_table_s4_t2_ipsec_txdma2_stats_update_d d;
struct phv_ p;

%%
        .align
esp_ipv4_tunnel_h2n_txdma2_ipsec_update_tx_stats:
    tbladd.f d.h2n_tx_pkts, 1
    //add r1, k.txdma2_global_payload_size, IPV4_FIXED_TUNNEL_MODE_GROWTH
    //add r1, r1, k.txdma2_global_pad_size
    //tbladd.f d.h2n_tx_bytes, r1 
    add r1, k.ipsec_to_stage4_ipsec_cb_addr, IPSEC_IP_HDR_OFFSET
    phvwr p.eth_hdr_dma_cmd_addr, r1

    phvwri p.{intrinsic_app_hdr_dma_cmd_phv_end_addr...intrinsic_app_hdr_dma_cmd_type}, ( ((CAPRI_PHV_END_OFFSET(p4_intr_global_tm_instance_type)) << 17) | (CAPRI_PHV_START_OFFSET(p4_intr_global_tm_iport) << 7) | CAPRI_DMA_COMMAND_PHV_TO_PKT)
    phvwri p.{ipsec_app_hdr_dma_cmd_phv_end_addr...ipsec_app_hdr_dma_cmd_type},  ((IPSEC_TXDMA2_APP_HEADER_END << 17) | (IPSEC_TXDMA2_APP_HEADER_START << 7) | CAPRI_DMA_COMMAND_PHV_TO_PKT)
    phvwri.e p.{app_header_table0_valid...app_header_table3_valid}, 0
    nop 

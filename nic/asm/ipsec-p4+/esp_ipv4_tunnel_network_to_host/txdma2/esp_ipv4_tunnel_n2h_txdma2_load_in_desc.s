#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s2_t0_k k;
struct tx_table_s2_t0_esp_v4_tunnel_n2h_txdma2_load_in_desc_d d;
struct phv_ p;

%%
        .param esp_v4_tunnel_n2h_txdma2_build_decap_packet 
        .param IPSEC_CB_BASE
        .align
esp_ipv4_tunnel_n2h_txdma2_load_in_desc:
    phvwr p.t0_s2s_in_page_addr, d.{addr0}.dx
     // Intrinsic
    phvwri p.{intrinsic_app_hdr_dma_cmd_phv_end_addr...intrinsic_app_hdr_dma_cmd_type}, ((CAPRI_PHV_END_OFFSET(p4_intr_global_tm_instance_type) << 17) | (CAPRI_PHV_START_OFFSET(p4_intr_global_tm_iport) << 7) | CAPRI_DMA_COMMAND_PHV_TO_PKT)
    phvwri p.p4_intr_global_tm_oport, TM_OPORT_P4INGRESS
    phvwri p.p4_intr_global_tm_iport, TM_OPORT_DMA
    phvwri p.p4_intr_global_lif, ARM_CPU_LIF
    phvwri p.ipsec_to_stage4_dot1q_etype, DOT1Q_ETYPE
    phvwri p.eth_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwri.e p.dec_pay_load_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    nop 

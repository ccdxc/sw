#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s2_t1_k k;
struct tx_table_s2_t1_ipsec_encap_txdma2_load_out_desc_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_h2n_txdma2_ipsec_build_encap_packet 
        .align
esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_out_desc:
    phvwr p.t0_s2s_out_page_addr, d.{addr0}.dx 
    //phvwri p.app_header_table1_valid, 0
    phvwri p.p4plus2p4_hdr_p4plus_app_id, P4PLUS_APPTYPE_IPSEC
    phvwri p.p4plus2p4_hdr_flags, P4PLUS_TO_P4_FLAGS_UPDATE_IP_LEN 

    // intrinsic
    phvwri p.intrinsic_app_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    phvwri p.intrinsic_app_hdr_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(p4_intr_global_tm_iport)
    phvwri p.intrinsic_app_hdr_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(p4_intr_global_tm_instance_type)
    // app-header
    phvwri p.ipsec_app_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    phvwri p.ipsec_app_hdr_dma_cmd_phv_start_addr, IPSEC_TXDMA2_APP_HEADER_START
    phvwri p.ipsec_app_hdr_dma_cmd_phv_end_addr,  IPSEC_TXDMA2_APP_HEADER_END

    nop.e
    nop
 

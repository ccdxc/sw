#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s2_t1_k k;
struct tx_table_s2_t1_esp_v4_tunnel_n2h_txdma2_load_out_desc_d d;
struct phv_ p;

%%
        .align
        .param esp_ipv4_tunnel_n2h_txdma2_load_pad_size_l4_proto
esp_ipv4_tunnel_n2h_txdma2_load_out_desc:
    phvwr p.t0_s2s_out_page_addr, d.{addr0}.dx
    add r2, d.{addr0}.dx, d.{length0}.wx
    subi r2, r2, 2
    phvwri p.{app_header_table0_valid...app_header_table1_valid}, 2

    phvwri p.{common_te0_phv_table_lock_en...common_te0_phv_table_raw_table_size}, 14 
    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_n2h_txdma2_load_pad_size_l4_proto[33:6]
    phvwr  p.common_te0_phv_table_addr, r2 

    phvwri p.p4_txdma_intr_dma_cmd_ptr, N2H_TXDMA2_DMA_COMMANDS_OFFSET
    phvwri p.p4plus2p4_hdr_flags, P4PLUS_TO_P4_FLAGS_UPDATE_IP_LEN

    phvwri p.ipsec_app_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    phvwri p.ipsec_app_hdr_dma_cmd_phv_start_addr, IPSEC_TXDMA2_APP_HEADER_START
    phvwri p.ipsec_app_hdr_dma_cmd_phv_end_addr, IPSEC_TXDMA2_APP_HEADER_END

    nop.e
    nop 

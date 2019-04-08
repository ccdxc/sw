#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s1_t0_k k;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_barco_req 
        .align
esp_ipv4_tunnel_h2n_txdma2_ipsec_dummy:
    phvwri p.p4plus2p4_hdr_p4plus_app_id, P4PLUS_APPTYPE_IPSEC
    phvwri p.p4plus2p4_hdr_flags, P4PLUS_TO_P4_FLAGS_UPDATE_IP_LEN
    phvwri p.p4_txdma_intr_dma_cmd_ptr, H2N_TXDMA2_DMA_COMMANDS_OFFSET
    phvwri p.ip_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwri p.ip_hdr_dma_cmd_cache, 1
    phvwri p.esp_iv_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwri p.esp_iv_hdr_dma_cmd_cache, 1
    add r5, r0, k.ipsec_to_stage1_barco_desc_addr
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_barco_req, r5, TABLE_SIZE_64_BITS)
    phvwri p.{p4_intr_global_tm_iport...p4_intr_global_tm_oport}, ((TM_OPORT_DMA << 4) | TM_OPORT_P4INGRESS) 
    phvwri.e p.p4_intr_global_lif, LIF_CPU
    nop

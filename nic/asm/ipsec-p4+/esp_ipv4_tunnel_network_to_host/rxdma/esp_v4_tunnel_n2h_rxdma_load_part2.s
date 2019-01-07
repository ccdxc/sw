#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct rx_table_s3_t2_k k;
struct rx_table_s3_t2_esp_v4_tunnel_n2h_load_part2_d d;
struct phv_ p;

%%
    .align
esp_v4_tunnel_n2h_rxdma_load_part2:
    phvwri p.app_header_table2_valid, 0
    phvwr p.ipsec_to_stage3_iv_salt, d.iv_salt
    tblwr.f d.last_replay_seq_no, k.t2_s2s_last_replay_seq_no
    phvwri p.p4_intr_global_tm_oport, TM_OPORT_P4INGRESS
    phvwri p.p4_intr_global_tm_iport, TM_OPORT_DMA
    phvwri p.p4_intr_global_lif, LIF_CPU
    phvwri p.{dma_cmd_pkt2mem_dma_cmd_cache...dma_cmd_pkt2mem_dma_cmd_type}, (IPSEC_PKT2MEM_CACHE_ENABLE | CAPRI_DMA_COMMAND_PKT_TO_MEM)
    phvwri p.{dma_cmd_pkt2mem2_dma_cmd_cache...dma_cmd_pkt2mem2_dma_cmd_type}, (IPSEC_PKT2MEM_CACHE_ENABLE | CAPRI_DMA_COMMAND_PKT_TO_MEM)
    phvwri.e p.{dma_cmd_iv_salt_dma_cmd_phv_end_addr...dma_cmd_iv_salt_dma_cmd_type}, ((IPSEC_IN_DESC_IV_SALT_END << 18) | (IPSEC_IN_DESC_IV_SALT_START << 8 ) | IPSEC_PHV2MEM_CACHE_ENABLE | CAPRI_DMA_COMMAND_PHV_TO_MEM)
    nop
    


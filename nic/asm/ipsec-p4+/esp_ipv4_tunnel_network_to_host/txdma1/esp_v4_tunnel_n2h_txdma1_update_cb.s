#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct tx_table_s4_t0_k k;
struct tx_table_s4_t0_esp_v4_tunnel_n2h_txdma1_update_cb_d d;
struct phv_ p;

%%
        .align
        .param IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
        .param esp_ipv4_tunnel_n2h_txdma1_ipsec_free_resources
esp_v4_tunnel_n2h_txdma1_update_cb:
    sne c1, k.txdma1_global_flags, 0
    bcf [c1], esp_ipv4_tunnel_n2h_hit_errors
    nop
    phvwri p.app_header_table0_valid, 0
    CAPRI_DMA_CMD_PHV2MEM_SETUP_I(dma_cmd_incr_pindex_dma_cmd, CAPRI_BARCO_MD_HENS_REG_GCM1_PRODUCER_IDX, barco_dbell_pi, barco_dbell_pi)
    CAPRI_DMA_CMD_STOP_FENCE(dma_cmd_incr_pindex_dma_cmd)
    nop.e 
    nop

esp_ipv4_tunnel_n2h_hit_errors:
    phvwri p.brq_req_write_dma_cmd_type, 0
    phvwri p.brq_in_desc_zero_dma_cmd_type, 0
    phvwri p.brq_out_desc_zero_dma_cmd_type, 0
    addi r5, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    addi r5, r5, IPSEC_N2H_SEM_CINDEX_OFFSET
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, esp_ipv4_tunnel_n2h_txdma1_ipsec_free_resources, r5, TABLE_SIZE_32_BITS)
    nop.e
    nop


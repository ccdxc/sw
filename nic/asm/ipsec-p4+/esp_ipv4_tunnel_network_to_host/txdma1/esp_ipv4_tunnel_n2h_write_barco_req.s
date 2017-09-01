#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct tx_table_s3_t0_k k;
struct tx_table_s3_t0_esp_v4_tunnel_n2h_txdma1_write_barco_req_d d;
struct phv_ p;

%%
        .param esp_v4_tunnel_n2h_txdma1_update_cb
        .align
esp_v4_tunnel_n2h_write_barco_req:
    add r1, r0, k.ipsec_to_stage3_barco_req_addr
    phvwri p.brq_req_write_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwr p.brq_req_write_dma_cmd_addr, r1
    phvwri p.brq_req_write_dma_cmd_phv_start_addr, IPSEC_TXDMA1_BARCO_REQ_PHV_OFFSET_START
    phvwri p.brq_req_write_dma_cmd_phv_end_addr, IPSEC_TXDMA1_BARCO_REQ_PHV_OFFSET_END 

    phvwri p.p4_txdma_intr_dma_cmd_ptr, N2H_TXDMA1_DMA_COMMANDS_OFFSET 
    phvwri p.app_header_table0_valid, 1
    addi r2, r0, esp_v4_tunnel_n2h_txdma1_update_cb
    srl r2, r2, 6
    phvwri p.common_te0_phv_table_lock_en, 1
    phvwri p.common_te0_phv_table_raw_table_size, 6
    phvwr p.common_te0_phv_table_addr, k.txdma1_global_ipsec_cb_addr

#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct tx_table_s1_t0_k k;
struct tx_table_s1_t0_esp_v4_tunnel_n2h_get_in_desc_from_cb_cindex_d d;
struct phv_ p;

%%
        .param esp_v4_tunnel_n2h_txdma1_load_head_desc_int_header
        .align
esp_v4_tunnel_n2h_get_in_desc_from_cb_cindex:

    phvwri p.brq_req_write_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri p.brq_req_write_dma_cmd_phv_start_addr, IPSEC_TXDMA1_BARCO_REQ_PHV_OFFSET_START
    phvwri p.brq_req_write_dma_cmd_phv_end_addr, IPSEC_TXDMA1_BARCO_REQ_PHV_OFFSET_END
    phvwri p.p4_txdma_intr_dma_cmd_ptr, N2H_TXDMA1_DMA_COMMANDS_OFFSET
    phvwri p.dma_cmd_post_barco_ring_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM


    phvwri p.{app_header_table0_valid...app_header_table1_valid}, 2
    phvwri p.{common_te0_phv_table_lock_en...common_te0_phv_table_raw_table_size}, 14 
    phvwri p.common_te0_phv_table_pc, esp_v4_tunnel_n2h_txdma1_load_head_desc_int_header[33:6] 
    phvwr.f p.common_te0_phv_table_addr, d.in_desc_addr
    nop.e
    nop

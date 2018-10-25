#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s3_t0_k k;
struct tx_table_s3_t0_ipsec_encap_txdma_load_head_desc_int_header_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_h2n_txdma1_ipsec_write_barco_req
        .align
esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_load_head_desc_int_header2:
    phvwri p.app_header_table1_valid, 0
    add r2, d.in_desc, 96
    add r3, d.out_desc, 96
    phvwr p.brq_in_desc_zero_dma_cmd_addr, r2 
    phvwri p.{brq_in_desc_zero_dma_cmd_phv_end_addr...brq_in_desc_zero_dma_cmd_type}, ((IPSEC_DESC_ZERO_CONTENT_END << 18) | (IPSEC_DESC_ZERO_CONTENT_START << 8) | CAPRI_DMA_COMMAND_PHV_TO_MEM)
    phvwr p.brq_out_desc_zero_dma_cmd_addr, r3 
    phvwri.e p.{brq_out_desc_zero_dma_cmd_phv_end_addr...brq_out_desc_zero_dma_cmd_type}, ((IPSEC_DESC_ZERO_CONTENT_END << 18) | (IPSEC_DESC_ZERO_CONTENT_START << 8) | CAPRI_DMA_COMMAND_PHV_TO_MEM)
    nop 


#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s2_t0_k k;
struct tx_table_s2_t0_ipsec_encap_txdma_load_head_desc_int_header_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_h2n_txdma1_ipsec_write_barco_req
        .align
esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_load_head_desc_int_header2:
    add r2, d.in_desc, 96
    add r3, d.out_desc, 96
    phvwri p.brq_in_desc_zero_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwr p.brq_in_desc_zero_dma_cmd_addr, r2 
    phvwri p.brq_in_desc_zero_dma_cmd_phv_start_addr, IPSEC_DESC_ZERO_CONTENT_START
    phvwri p.brq_in_desc_zero_dma_cmd_phv_end_addr, IPSEC_DESC_ZERO_CONTENT_END
    phvwri p.brq_out_desc_zero_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwr p.brq_out_desc_zero_dma_cmd_addr, r3 
    phvwri p.brq_out_desc_zero_dma_cmd_phv_start_addr, IPSEC_DESC_ZERO_CONTENT_START
    phvwri.f p.brq_out_desc_zero_dma_cmd_phv_end_addr, IPSEC_DESC_ZERO_CONTENT_END
    nop.e
    nop 


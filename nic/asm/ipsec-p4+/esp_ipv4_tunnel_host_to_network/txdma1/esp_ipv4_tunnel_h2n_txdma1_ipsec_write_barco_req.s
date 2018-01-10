#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct tx_table_s3_t0_k k;
struct tx_table_s3_t0_ipsec_write_barco_req_d d;
struct phv_ p;

%%
        .align
esp_ipv4_tunnel_h2n_txdma1_ipsec_write_barco_req:
    add r1, r0, k.ipsec_to_stage3_barco_req_addr
    phvwri p.brq_req_write_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwr p.brq_req_write_dma_cmd_addr, r1
    phvwri p.brq_req_write_dma_cmd_phv_start_addr, IPSEC_TXDMA1_BARCO_REQ_PHV_OFFSET_START
    phvwri p.brq_req_write_dma_cmd_phv_end_addr, IPSEC_TXDMA1_BARCO_REQ_PHV_OFFSET_END 

esp_ipv4_tunnel_h2n_txdma1_ipsec_ring_barco_doorbell:
    phvwri p.dma_cmd_incr_pindex_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri p.dma_cmd_incr_pindex_dma_cmd_addr, CAPRI_BARCO_MD_HENS_REG_GCM0_PRODUCER_IDX
    phvwri p.dma_cmd_incr_pindex_dma_cmd_phv_start_addr, IPSEC_BARCO_DOORBELL_OFFSET_START
    phvwri p.dma_cmd_incr_pindex_dma_cmd_phv_end_addr, IPSEC_BARCO_DOORBELL_OFFSET_END
    phvwri p.dma_cmd_incr_pindex_dma_cmd_eop, 1
    phvwri p.dma_cmd_incr_pindex_dma_cmd_wr_fence, 1

esp_ipv4_tunnel_h2n_post_to_barco_ring:
    add r2, r0, d.barco_ring_base_addr_hi, 32
    add r2, r2, d.barco_ring_base_addr
    add r3, r0, d.barco_pindex
    andi r3, r3, IPSEC_BARCO_RING_INDEX_MASK
    sll r3, r3, IPSEC_BARCO_RING_ENTRY_SHIFT_SIZE
    add r3, r3, r2
    phvwri p.dma_cmd_post_barco_ring_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwr p.dma_cmd_post_barco_ring_dma_cmd_addr, r3
    phvwri p.dma_cmd_post_barco_ring_dma_cmd_phv_start_addr, IPSEC_TXDMA1_BARCO_REQ_PHV_OFFSET_START
    phvwri p.dma_cmd_post_barco_ring_dma_cmd_phv_end_addr, IPSEC_TXDMA1_BARCO_REQ_PHV_OFFSET_END

esp_ipv4_tunnel_h2n_dma_cmd_incr_barco_pindex:
    tbladd d.barco_pindex, 1
    tbland d.barco_pindex, 0x3F
    phvwri p.p4_txdma_intr_dma_cmd_ptr, H2N_TXDMA1_DMA_COMMANDS_OFFSET 
    phvwri p.app_header_table0_valid, 0
    phvwri p.app_header_table1_valid, 0
    phvwri p.app_header_table2_valid, 0
    phvwri p.app_header_table3_valid, 0
    nop.e
    nop

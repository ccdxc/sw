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
    phvwr p.brq_req_write_dma_cmd_addr, k.ipsec_to_stage3_barco_req_addr 
    phvwri p.brq_req_write_dma_cmd_phv_start_addr, IPSEC_TXDMA1_BARCO_REQ_PHV_OFFSET_START
    phvwri p.brq_req_write_dma_cmd_phv_end_addr, IPSEC_TXDMA1_BARCO_REQ_PHV_OFFSET_END 

esp_ipv4_tunnel_h2n_post_to_barco_ring:
    and r3, d.barco_pindex, IPSEC_BARCO_RING_INDEX_MASK 
    sll r3, r3, IPSEC_BARCO_RING_ENTRY_SHIFT_SIZE
    add r3, r3, d.barco_ring_base_addr 
    phvwr p.dma_cmd_post_barco_ring_dma_cmd_addr, r3
    phvwri p.dma_cmd_post_barco_ring_dma_cmd_phv_start_addr, IPSEC_TXDMA1_BARCO_REQ_PHV_OFFSET_START
    phvwri p.dma_cmd_post_barco_ring_dma_cmd_phv_end_addr, IPSEC_TXDMA1_BARCO_REQ_PHV_OFFSET_END

esp_ipv4_tunnel_h2n_dma_cmd_incr_barco_pindex:
    add r7, d.barco_pindex, 1
    and r7, r7, IPSEC_BARCO_RING_INDEX_MASK 
    tblwr.f d.barco_pindex, r7 
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0

esp_ipv4_tunnel_h2n_txdma1_ipsec_ring_barco_doorbell:
    addi r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_SET, DB_SCHED_UPD_SET, 0, LIF_IPSEC_ESP)
    phvwr p.barco_req_doorbell_address, r4.dx
    CAPRI_RING_DOORBELL_DATA(0, d.ipsec_cb_index, 1, r7)
    phvwr p.barco_req_doorbell_data, r3.dx

    CAPRI_DMA_CMD_PHV2MEM_SETUP_I(dma_cmd_incr_pindex_dma_cmd, CAPRI_BARCO_MD_HENS_REG_GCM0_PRODUCER_IDX, barco_dbell_pi, barco_dbell_pi)
    CAPRI_DMA_CMD_STOP_FENCE(dma_cmd_incr_pindex_dma_cmd)
    phvwri p.dma_cmd_incr_pindex_dma_cmd_eop, 1

esp_ipv4_tunnel_h2n_txdma1_ipsec_write_barco_req_do_nothing:
    nop.e
    nop

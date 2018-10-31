#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct tx_table_s4_t0_k k;
struct tx_table_s4_t0_ipsec_write_barco_req_d d;
struct phv_ p;

%%
        .align
esp_ipv4_tunnel_h2n_txdma1_ipsec_write_barco_req:

esp_ipv4_tunnel_h2n_post_to_barco_ring:
    and r3, d.barco_pindex, IPSEC_BARCO_RING_INDEX_MASK 
    add r7, d.barco_pindex, 1
    and r7, r7, IPSEC_BARCO_RING_INDEX_MASK 
    tblwr.f d.barco_pindex, r7 
    sll r3, r3, IPSEC_BARCO_RING_ENTRY_SHIFT_SIZE
    add r3, r3, d.barco_ring_base_addr 
    blti  r3, CAPRI_HBM_BASE, esp_ipv4_tunnel_h2n_txdma1_ipsec_write_barco_req_illegal_dma 
    phvwr p.dma_cmd_post_barco_ring_dma_cmd_addr, r3

esp_ipv4_tunnel_h2n_dma_cmd_incr_barco_pindex:
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0

esp_ipv4_tunnel_h2n_txdma1_ipsec_ring_barco_doorbell:
    addi r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_SET, DB_SCHED_UPD_SET, 0, LIF_IPSEC_ESP)
    phvwr p.barco_req_doorbell_address, r4.dx
    CAPRI_RING_DOORBELL_DATA(0, d.ipsec_cb_index, 1, r7)
    phvwr p.barco_req_doorbell_data, r3.dx

    add r2, r0, k.ipsec_to_stage4_barco_req_addr
    blti r2, CAPRI_HBM_BASE, esp_ipv4_tunnel_h2n_txdma1_ipsec_write_barco_req_illegal_dma
    phvwr p.brq_req_write_dma_cmd_addr, k.ipsec_to_stage4_barco_req_addr 

    CAPRI_DMA_CMD_PHV2MEM_SETUP_I(dma_cmd_incr_pindex_dma_cmd, CAPRI_BARCO_MD_HENS_REG_GCM0_PRODUCER_IDX, barco_dbell_pi, barco_dbell_pi)
    CAPRI_DMA_CMD_STOP_FENCE(dma_cmd_incr_pindex_dma_cmd)
    phvwri.e p.dma_cmd_incr_pindex_dma_cmd_eop, 1
    nop

esp_ipv4_tunnel_h2n_txdma1_ipsec_write_barco_req_illegal_dma:
    phvwri p.p4_intr_global_drop, 1
    CAPRI_CLEAR_TABLE0_VALID
    CAPRI_CLEAR_TABLE1_VALID
    CAPRI_CLEAR_TABLE2_VALID
    CAPRI_CLEAR_TABLE3_VALID
    nop.e
    nop


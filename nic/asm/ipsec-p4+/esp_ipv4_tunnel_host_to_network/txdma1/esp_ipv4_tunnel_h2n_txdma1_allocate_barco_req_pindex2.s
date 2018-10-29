#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"


struct tx_table_s2_t1_allocate_barco_req_pindex_d d;
struct tx_table_s2_t1_k k;
struct phv_ p;

%%
       .param BRQ_BASE 
       .align
esp_ipv4_tunnel_h2n_txdma1_allocate_barco_req_pindex2:
    add r4, d.pi, 1
    andi r4, r4, IPSEC_BARCO_RING_INDEX_MASK
    seq c1, r4, d.ci 
    bcf [c1], esp_ipv4_tunnel_h2n_txdma1_allocate_barco_req_fail
    sll r2, d.pi, BRQ_RING_ENTRY_SIZE_SHIFT 
    addui r2, r2, hiword(BRQ_BASE)
    addi r2, r2, loword(BRQ_BASE)
    phvwr p.ipsec_to_stage4_barco_req_addr, r2
    tblwr d.pi, r4
    phvwr p.barco_dbell_pi, r4.wx
    phvwri p.brq_req_write_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri p.dma_cmd_post_barco_ring_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri.e p.p4_txdma_intr_dma_cmd_ptr, H2N_TXDMA1_DMA_COMMANDS_OFFSET
    nop

esp_ipv4_tunnel_h2n_txdma1_allocate_barco_req_fail:
    tbladd d.stat_qfull, 1
    phvwri p.p4_intr_global_drop, 1
    nop.e
    nop
    

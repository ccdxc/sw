#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"


/* d is the data returned by lookup result */
struct d_struct {
    barco_pi  : 16;
    pad       : 496;
};

struct d_struct d;
struct tx_table_s2_t1_k k;
struct phv_ p;

%%
       .param BRQ_BASE 
       .align
esp_ipv4_tunnel_h2n_txdma1_allocate_barco_req_pindex2:
    and r2, d.barco_pi, IPSEC_BARCO_RING_INDEX_MASK 
    sll r2, r2, BRQ_RING_ENTRY_SIZE_SHIFT 
    addui r2, r2, hiword(BRQ_BASE)
    addi r2, r2, loword(BRQ_BASE)
    phvwr p.ipsec_to_stage4_barco_req_addr, r2
    add r1, d.barco_pi, 1
    and r1, r1, IPSEC_BARCO_RING_INDEX_MASK 
    tblwr d.barco_pi, r1
    phvwr p.barco_dbell_pi, r1.wx
    phvwri p.brq_req_write_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri p.dma_cmd_post_barco_ring_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri.f p.p4_txdma_intr_dma_cmd_ptr, H2N_TXDMA1_DMA_COMMANDS_OFFSET
    nop.e
    nop
    

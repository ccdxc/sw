#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s1_t1_k k;
struct tx_table_s1_t1_allocate_barco_req_pindex_d d;
struct phv_ p;

%%
       .param BRQ_BASE 
        .align
esp_ipv4_tunnel_h2n_txdma1_allocate_barco_req_pindex:
    phvwri p.app_header_table1_valid, 0
    sll r2, d.{pi}.wx, BRQ_RING_ENTRY_SIZE_SHIFT 
    addui r2, r2, hiword(BRQ_BASE)
    addi r2, r2, loword(BRQ_BASE)
    phvwr p.ipsec_to_stage3_barco_req_addr, r2
    add r1, d.{pi}.wx, 1
    phvwr p.barco_dbell_pi, r1.wx
    phvwri p.p4_txdma_intr_dma_cmd_ptr, H2N_TXDMA1_DMA_COMMANDS_OFFSET
    nop.e
    nop
    

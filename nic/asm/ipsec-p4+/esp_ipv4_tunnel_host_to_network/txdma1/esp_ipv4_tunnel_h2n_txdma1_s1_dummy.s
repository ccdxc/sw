#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s1_t0_k k;
struct tx_table_s1_t0_tx_table_dummy_action_d d;
struct phv_ p;

%%
       .param      esp_ipv4_tunnel_h2n_txdma1_ipsec_get_in_desc_from_cb_cindex
       .align
esp_ipv4_tunnel_h2n_txdma1_s1_dummy:
    add r4, r0, k.ipsec_to_stage1_cb_ring_slot_addr
    phvwri p.p4_txdma_intr_dma_cmd_ptr, H2N_TXDMA1_DMA_COMMANDS_OFFSET
    phvwri p.barco_req_header_size, ESP_FIXED_HDR_SIZE_LI
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, esp_ipv4_tunnel_h2n_txdma1_ipsec_get_in_desc_from_cb_cindex, r4, TABLE_SIZE_256_BITS) 
    nop.e
    nop

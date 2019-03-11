#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"

struct rx_table_s3_t1_k k;
struct rx_table_s3_t1_rx_table_s3_t1_cfg_action_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc
        .param esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc2
        .param esp_ipv4_tunnel_h2n_rxdma_ipsec_update_rx_stats
        .align

esp_ipv4_tunnel_h2n_update_output_desc_aol:
    add r6, k.t1_s2s_payload_size, k.ipsec_to_stage3_pad_size
    addi r6, r6, ESP_FIXED_HDR_SIZE+2
    phvwr p.barco_desc_out_L0, r6.wx 
    add r4, k.t1_s2s_ipsec_cb_addr, IPSEC_H2N_STATS_CB_OFFSET
    add r1, r0, k.t1_s2s_ipsec_cb_addr 
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc, r1, TABLE_SIZE_512_BITS)
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_EN, esp_ipv4_tunnel_h2n_rxdma_ipsec_update_rx_stats, r4, TABLE_SIZE_512_BITS)
    CAPRI_NEXT_TABLE_READ(2, TABLE_LOCK_DIS, esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc2, r1, TABLE_SIZE_512_BITS)
    nop.e
    nop

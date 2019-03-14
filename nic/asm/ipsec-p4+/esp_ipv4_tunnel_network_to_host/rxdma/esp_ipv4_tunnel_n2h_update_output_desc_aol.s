#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"

struct rx_table_s3_t1_k k;
struct rx_table_s3_t1_rx_table_s3_t1_cfg_action_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc
        .param esp_ipv4_tunnel_n2h_rxdma_ipsec_update_rx_stats
        .align

esp_ipv4_tunnel_n2h_update_output_desc_aol:
    phvwri p.{app_header_table0_valid...app_header_table1_valid}, 3
    add r6, k.t1_s2s_out_desc_addr, IPSEC_PAGE_OFFSET
    phvwr p.barco_desc_out_A0_addr, r6.dx 
    phvwr p.ipsec_int_header_out_page, r6 
    add r2, k.ipsec_to_stage3_payload_size, ESP_FIXED_HDR_SIZE
    phvwr p.barco_desc_out_L0, r2.wx 
    add r2, r0, k.ipsec_to_stage3_ipsec_cb_addr
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc, r2, TABLE_SIZE_512_BITS)
    //add r1, k.ipsec_to_stage3_ipsec_cb_addr, IPSEC_N2H_STATS_CB_OFFSET
    //CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_EN, esp_ipv4_tunnel_n2h_rxdma_ipsec_update_rx_stats, r1, TABLE_SIZE_512_BITS)
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(1, esp_ipv4_tunnel_n2h_rxdma_ipsec_update_rx_stats)
    nop.e
    nop

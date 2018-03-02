#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"

struct rx_table_s3_t1_k k;
struct rx_table_s3_t1_rx_table_s3_t1_cfg_action_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc
        .param esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc2
        .align

esp_ipv4_tunnel_h2n_update_output_desc_aol:
    phvwr p.barco_desc_out_A0_addr, k.{t1_s2s_out_page_addr}.dx 
    add r6, k.t1_s2s_payload_size, k.ipsec_to_stage3_pad_size
    addi r6, r6, ESP_FIXED_HDR_SIZE+2
    phvwr p.barco_desc_out_L0, r6.wx 
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 10 
    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc[33:6]
    phvwri p.{common_te0_phv_table_lock_en...common_te0_phv_table_raw_table_size}, ((1 << 3) | 6 ) 
    phvwr p.common_te0_phv_table_addr, k.{ipsec_global_ipsec_cb_addr_sbit0_ebit31...ipsec_global_ipsec_cb_addr_sbit32_ebit33}
    phvwri p.common_te2_phv_table_pc, esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc2[33:6]
    phvwr p.common_te2_phv_table_addr, k.{ipsec_global_ipsec_cb_addr_sbit0_ebit31...ipsec_global_ipsec_cb_addr_sbit32_ebit33}
    phvwri.f p.{common_te2_phv_table_lock_en...common_te2_phv_table_raw_table_size}, ((1 << 3) | 6 ) 
    nop.e
    nop

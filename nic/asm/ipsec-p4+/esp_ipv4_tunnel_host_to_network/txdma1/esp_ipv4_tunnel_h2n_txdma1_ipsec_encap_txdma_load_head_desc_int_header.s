#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s2_t0_k k;
struct tx_table_s2_t0_ipsec_encap_txdma_load_head_desc_int_header_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_h2n_txdma1_ipsec_write_barco_req
        .align
esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_load_head_desc_int_header:
    phvwr p.barco_req_input_list_address, d.in_desc
    phvwr p.barco_req_output_list_address, d.out_desc
    phvwr p.barco_req_iv_address, d.in_page
    add r1, r0, d.pad_size
    add r1, r1, d.tailroom_offset
    addi r1, r1, 2
    add r1, r1, d.out_page
    phvwr p.barco_req_auth_tag_addr, r1
    add r3, r0, k.t0_s2s_iv_size
    addi r3, r3, ESP_FIXED_HDR_SIZE 
    phvwr p.barco_req_header_size, r3 

    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_h2n_txdma1_ipsec_write_barco_req[33:6] 
    phvwri p.common_te0_phv_table_lock_en, 1
    phvwri p.common_te0_phv_table_raw_table_size, 6
    phvwr  p.common_te0_phv_table_addr, k.ipsec_to_stage2_barco_req_addr 
    phvwri p.app_header_table1_valid, 0

    nop.e
    nop 


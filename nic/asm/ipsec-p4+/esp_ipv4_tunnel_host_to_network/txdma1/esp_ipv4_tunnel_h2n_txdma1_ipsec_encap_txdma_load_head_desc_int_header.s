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
    add r2, d.in_desc, 56
    add r3, d.in_desc, 64
    phvwr p.barco_req_input_list_address, r3.dx
    phvwr p.barco_req_status_address, r2.dx
    add r1, d.out_desc, 64
    phvwr p.barco_req_output_list_address, r1.dx
    phvwr p.barco_req_iv_address, d.{in_page}.dx 
    add r1, d.pad_size, d.tailroom_offset
    addi r1, r1, ESP_FIXED_HDR_SIZE+2
    add r1, r1, d.out_page
    phvwr p.barco_req_auth_tag_addr, r1.dx
    phvwri p.barco_req_header_size, ESP_FIXED_HDR_SIZE_LI 
    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_h2n_txdma1_ipsec_write_barco_req[33:6] 
    phvwri p.{common_te0_phv_table_lock_en...common_te0_phv_table_raw_table_size}, 14
    phvwr.f  p.common_te0_phv_table_addr, k.txdma1_global_ipsec_cb_addr
    nop.e
    nop 


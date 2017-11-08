#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"

struct rx_table_s3_t1_k k;
struct rx_table_s3_t1_rx_table_s3_t1_cfg_action_d d;
struct phv_ p;

%%
        .align

esp_ipv4_tunnel_h2n_update_output_desc_aol:
    phvwri p.app_header_table1_valid, 0
    add r1, r0, k.t1_s2s_out_page_addr
    phvwr p.barco_desc_out_A0_addr, r1.dx 
    // get the correct way of giving it as a single 14 bit field
    add r6, r0, k.t1_s2s_payload_size
    add r6, r6, k.ipsec_to_stage3_pad_size
    addi r6, r6, ESP_FIXED_HDR_SIZE
    add r6, r6, k.ipsec_to_stage3_iv_size
    addi r6, r6, 2
    phvwr p.barco_desc_out_L0, r6.wx 
    //phvwri p.barco_desc_out_O0, 0
    nop.e
    nop

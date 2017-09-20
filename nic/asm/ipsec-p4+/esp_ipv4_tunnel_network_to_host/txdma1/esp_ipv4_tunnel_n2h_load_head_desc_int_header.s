#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s2_t1_k k;
struct tx_table_s2_t1_esp_v4_tunnel_n2h_txdma1_load_head_desc_int_header_d d;
struct phv_ p;

%%
        .align
esp_v4_tunnel_n2h_txdma1_load_head_desc_int_header:
    phvwr p.barco_req_input_list_address, d.in_desc
    phvwr p.barco_req_output_list_address, d.out_desc
    add r1, r0, d.pad_size
    add r1, r1, d.tailroom_offset
    addi r1, r1, 2
    phvwr p.barco_req_auth_tag_addr, r1
    phvwr p.barco_req_header_size, d.payload_start



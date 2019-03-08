#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct common_p4plus_stage0_app_header_table_esp_v4_tunnel_n2h_rxdma_initial_table_d d;
struct common_p4plus_stage0_app_header_table_k k;
struct phv_ p;

%%
        .param          esp_ipv4_tunnel_n2h_allocate_input_desc_semaphore
        .param          esp_ipv4_tunnel_n2h_allocate_output_desc_semaphore
        .param          esp_ipv4_tunnel_n2h_allocate_input_page_semaphore
        .param          esp_ipv4_tunnel_n2h_allocate_output_page_semaphore
        .align
ipsec_esp_v4_tunnel_n2h_good_pkt:

    addi r1, r0, INDESC_SEMAPHORE_ADDR
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, esp_ipv4_tunnel_n2h_allocate_input_desc_semaphore, r1, TABLE_SIZE_64_BITS)

    phvwr p.ipsec_int_header_ipsec_cb_index, d.ipsec_cb_index
    phvwr p.ipsec_global_ipsec_cb_index, d.ipsec_cb_index
    phvwr.e p.ipsec_to_stage3_iv_size, d.iv_size
    nop


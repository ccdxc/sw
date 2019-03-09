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
    nop.e
    nop


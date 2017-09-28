#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"

struct rx_table_s2_t3_k k;
struct rx_table_s2_t3_esp_v4_tunnel_n2h_allocate_output_page_index_d d;
struct phv_ p;

%%
        .align
esp_v4_tunnel_n2h_allocate_output_page_index:
    phvwri p.app_header_table3_valid, 0
    phvwr p.t1_s2s_out_page_addr, d.out_page_index
    phvwr p.ipsec_int_header_out_page, d.out_page_index
    nop.e 


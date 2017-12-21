#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s1_t2_k k;
struct tx_table_s1_t2_esp_v4_tunnel_n2h_load_part2_d d;
struct phv_ p;

%%
    .align
esp_v4_tunnel_n2h_load_part2:
    phvwr p.ipsec_to_stage2_spi, d.spi
    phvwr p.ipsec_to_stage2_new_spi, d.new_spi
    phvwri p.app_header_table2_valid, 0
    nop.e
    nop 

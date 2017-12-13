#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct rx_table_s3_t2_k k;
struct rx_table_s3_t2_read_random_number_from_barco_d d;
struct phv_ p;

%%
    .align

esp_ipv4_tunnel_h2n_read_random_number_from_barco:
    phvwr p.esp_header_iv, d.random_number1
    phvwr p.esp_header_iv2, d.random_number2
    nop.e
    nop

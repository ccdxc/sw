#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct tx_table_s1_t0_k k;
struct tx_table_s1_t0_esp_v4_tunnel_n2h_get_in_desc_from_cb_cindex_d d;
struct phv_ p;

%%
        .param esp_v4_tunnel_n2h_txdma1_load_head_desc_int_header
        .align
esp_v4_tunnel_n2h_get_in_desc_from_cb_cindex:

    phvwri p.app_header_table1_valid, 0
    phvwri p.app_header_table0_valid, 1
    phvwri p.common_te0_phv_table_lock_en, 1
    phvwri p.common_te0_phv_table_pc, esp_v4_tunnel_n2h_txdma1_load_head_desc_int_header[33:6] 
    phvwri p.common_te0_phv_table_raw_table_size, 6
    phvwr.e p.common_te0_phv_table_addr, d.in_desc_addr
    nop

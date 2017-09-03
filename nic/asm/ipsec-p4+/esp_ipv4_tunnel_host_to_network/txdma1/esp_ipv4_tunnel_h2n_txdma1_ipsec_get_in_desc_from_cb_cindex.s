#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct tx_table_s1_t0_k k;
struct tx_table_s1_t0_ipsec_get_in_desc_from_cb_cindex_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_load_head_desc_int_header 
        .align
esp_ipv4_tunnel_h2n_txdma1_ipsec_get_in_desc_from_cb_cindex:
    phvwr p.txdma1_global_in_desc_addr, d.in_desc_addr
    phvwri p.app_header_table0_valid, 1
    phvwri p.common_te0_phv_table_lock_en, 1
    phvwri p.common_te0_phv_table_raw_table_size, 6
    addi r2, r0, esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_load_head_desc_int_header
    srl r2, r2, 6
    phvwr p.common_te0_phv_table_pc, r2
    phvwr p.common_te0_phv_table_addr, d.in_desc_addr
    

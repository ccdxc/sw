#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"
#include "tls-macros.h"

struct rx_table_s2_t2_k k;
struct rx_table_s2_t2_allocate_input_page_index_d d;
struct phv_ p;

%%
        

    .param esp_ipv4_tunnel_h2n_read_random_number_from_barco 
    .align
esp_ipv4_tunnel_h2n_allocate_input_page_index:
    phvwr p.t0_s2s_in_page_addr, d.in_page_index
    phvwr p.t2_s2s_in_page_addr, d.in_page_index
    phvwr p.t3_s2s_in_page_addr, d.in_page_index
    phvwr p.ipsec_int_header_in_page, d.in_page_index 
    seq c1, k.ipsec_to_stage2_is_random, 1 
    bcf [c1], esp_ipv4_tunnel_h2n_enable_read_random
    phvwr.!c1 p.app_header_table2_valid, 0
    nop.e
    nop


esp_ipv4_tunnel_h2n_enable_read_random:
    CAPRI_BARCO_DRBG_RANDOM0_GENERATE(r1, r2)
    phvwri p.app_header_table2_valid, 1
    phvwri p.common_te2_phv_table_pc, esp_ipv4_tunnel_h2n_read_random_number_from_barco[33:6]
    // Will change to 3 later - model has a bug and cannot pull more than 32 bits right now
    phvwri p.{common_te2_phv_table_lock_en...common_te2_phv_table_raw_table_size}, 10 
    phvwri p.common_te2_phv_table_lock_en, 1
    phvwri p.common_te2_phv_table_addr, CAPRI_BARCO_MD_HENS_REG_DRBG_RANDOM_NUM0 
    nop.e 
    nop

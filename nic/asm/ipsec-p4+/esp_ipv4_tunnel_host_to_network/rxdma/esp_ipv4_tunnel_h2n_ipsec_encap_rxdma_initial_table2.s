#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct common_p4plus_stage0_app_header_table_ipsec_encap_rxdma_initial_table_d d;
struct common_p4plus_stage0_app_header_table_k k;
struct phv_ p;

%%
        .param          esp_ipv4_tunnel_h2n_allocate_input_desc_semaphore
        .param          esp_ipv4_tunnel_h2n_allocate_output_desc_semaphore
        .param          esp_ipv4_tunnel_h2n_allocate_input_page_semaphore
        .param          esp_ipv4_tunnel_h2n_allocate_output_page_semaphore
        .align
esp_ipv4_tunnel_h2n_ipsec_encap_rxdma_initial_table2:
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 15
    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_h2n_allocate_input_desc_semaphore[33:6] 
    phvwri p.{common_te0_phv_table_lock_en...common_te0_phv_table_raw_table_size}, 11 
    phvwri p.common_te0_phv_table_addr, INDESC_SEMAPHORE_ADDR

    phvwri p.common_te1_phv_table_pc, esp_ipv4_tunnel_h2n_allocate_output_desc_semaphore[33:6] 
    phvwri p.{common_te1_phv_table_lock_en...common_te1_phv_table_raw_table_size}, 11 
    phvwri p.common_te1_phv_table_addr, OUTDESC_SEMAPHORE_ADDR
  
    phvwri p.common_te2_phv_table_pc, esp_ipv4_tunnel_h2n_allocate_input_page_semaphore[33:6] 
    phvwri p.{common_te2_phv_table_lock_en...common_te2_phv_table_raw_table_size}, 11 
    phvwri p.common_te2_phv_table_addr, INPAGE_SEMAPHORE_ADDR

    phvwri p.common_te3_phv_table_pc, esp_ipv4_tunnel_h2n_allocate_output_page_semaphore[33:6] 
    phvwri p.{common_te3_phv_table_lock_en...common_te3_phv_table_raw_table_size}, 11 
    phvwri.f p.common_te3_phv_table_addr, OUTPAGE_SEMAPHORE_ADDR
    nop.e
    nop

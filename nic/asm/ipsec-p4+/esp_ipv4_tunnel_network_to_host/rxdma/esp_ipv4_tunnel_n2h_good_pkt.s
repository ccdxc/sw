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
    phvwri p.app_header_table0_valid, 1
    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_n2h_allocate_input_desc_semaphore[33:6]
    phvwri p.common_te0_phv_table_raw_table_size, 3
    phvwri p.common_te0_phv_table_lock_en, 0
    phvwri p.common_te0_phv_table_addr, INDESC_SEMAPHORE_ADDR

    phvwri p.app_header_table1_valid, 1
    phvwri p.common_te1_phv_table_pc, esp_ipv4_tunnel_n2h_allocate_output_desc_semaphore[33:6]
    phvwri p.common_te1_phv_table_raw_table_size, 3
    phvwri p.common_te1_phv_table_lock_en, 0
    phvwri p.common_te1_phv_table_addr, OUTDESC_SEMAPHORE_ADDR

    phvwri p.app_header_table2_valid, 1
    phvwri p.common_te2_phv_table_pc, esp_ipv4_tunnel_n2h_allocate_input_page_semaphore[33:6]
    phvwri p.common_te2_phv_table_raw_table_size, 3
    phvwri p.common_te2_phv_table_lock_en, 0
    phvwri p.common_te2_phv_table_addr, INPAGE_SEMAPHORE_ADDR

    phvwri p.app_header_table3_valid, 1
    phvwri p.common_te3_phv_table_pc, esp_ipv4_tunnel_n2h_allocate_output_page_semaphore[33:6]
    phvwri p.common_te3_phv_table_raw_table_size, 3
    phvwri p.common_te3_phv_table_lock_en, 0
    phvwri p.common_te3_phv_table_addr, OUTPAGE_SEMAPHORE_ADDR
    nop.e
    nop


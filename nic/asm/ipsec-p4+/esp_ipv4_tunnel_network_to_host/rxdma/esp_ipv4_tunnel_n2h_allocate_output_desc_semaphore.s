#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"

struct rx_table_s1_t1_k k;
struct rx_table_s1_t1_esp_v4_tunnel_n2h_allocate_output_desc_semaphore_d d;
struct phv_ p;

%%
        .param          esp_ipv4_tunnel_n2h_allocate_output_desc_index 
        .param          TNMDR_TABLE_BASE
        .align

esp_ipv4_tunnel_n2h_allocate_output_desc_semaphore:
    phvwri p.app_header_table1_valid, 1
    phvwri p.common_te1_phv_table_pc, esp_ipv4_tunnel_n2h_allocate_output_desc_index[33:6] 
    phvwri p.{common_te1_phv_table_lock_en...common_te1_phv_table_raw_table_size}, 11 
    sll r1, d.{out_desc_ring_index}.dx, 3 
    addui r1, r1, hiword(TNMDR_TABLE_BASE)
    addi r1, r1, loword(TNMDR_TABLE_BASE)
    phvwr p.common_te1_phv_table_addr, r1
    nop.e 
    nop


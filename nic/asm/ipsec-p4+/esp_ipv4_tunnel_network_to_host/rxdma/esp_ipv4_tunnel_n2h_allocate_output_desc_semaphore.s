#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_defines.h"

struct rx_table_s1_t1_k k;
struct rx_table_s1_t1_esp_v4_tunnel_n2h_allocate_output_desc_semaphore_d d;
struct phv_ p;

%%
        .param          esp_ipv4_tunnel_n2h_allocate_output_desc_index 
        .align

esp_ipv4_tunnel_n2h_allocate_output_desc_semaphore:
    phvwri p.p42p4plus_hdr_table1_valid, 1
    addi r2, r0, esp_ipv4_tunnel_n2h_allocate_output_desc_index
    srl r2, r2, 6
    phvwr p.common_te1_phv_table_pc, r2 
    phvwri p.common_te1_phv_table_raw_table_size, 3
    phvwri p.common_te1_phv_table_lock_en, 0
    sll r1, d.out_desc_ring_index, 3 
    addi r1, r1, OUT_DESC_RING_BASE 
    phvwr p.common_te1_phv_table_addr, r1
    nop.e 


#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"

struct rx_table_s1_t0_k k;
struct rx_table_s1_t0_esp_v4_tunnel_n2h_allocate_input_desc_semaphore_d d;
struct phv_ p;

%%
        .param          esp_ipv4_tunnel_n2h_allocate_input_desc_index 
        .param          RNMDR_TABLE_BASE
        .align

esp_ipv4_tunnel_n2h_allocate_input_desc_semaphore:
    phvwri p.app_header_table0_valid, 1
    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_n2h_allocate_input_desc_index[33:6] 
    phvwri p.{common_te0_phv_table_lock_en...common_te0_phv_table_raw_table_size}, 11 
    and r1, d.{in_desc_ring_index}.dx, 0x3FF
    sll r1, r1, 3 
    addui r1, r1, hiword(RNMDR_TABLE_BASE)
    addi r1, r1, loword(RNMDR_TABLE_BASE)
    phvwr p.common_te0_phv_table_addr, r1

    phvwri p.dma_cmd_phv2mem_ipsec_int_dma_cmd_phv_start_addr, IPSEC_N2H_INT_START_OFFSET
    phvwri p.dma_cmd_phv2mem_ipsec_int_dma_cmd_phv_end_addr, IPSEC_N2H_INT_END_OFFSET
    phvwri p.dma_cmd_in_desc_aol_dma_cmd_phv_start_addr, IPSEC_IN_DESC_AOL_START
    phvwri p.dma_cmd_in_desc_aol_dma_cmd_phv_end_addr, IPSEC_IN_DESC_AOL_END
    phvwri p.dma_cmd_out_desc_aol_dma_cmd_phv_start_addr, IPSEC_OUT_DESC_AOL_START
    phvwri p.dma_cmd_out_desc_aol_dma_cmd_phv_end_addr, IPSEC_OUT_DESC_AOL_END

    nop.e
    nop
     


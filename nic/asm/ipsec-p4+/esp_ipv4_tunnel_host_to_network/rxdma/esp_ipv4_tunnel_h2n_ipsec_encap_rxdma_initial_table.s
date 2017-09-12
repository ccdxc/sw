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
        .param          IPSEC_PAD_BYTES_HBM_TABLE_BASE
        .align 
esp_ipv4_tunnel_h2n_ipsec_encap_rxdma_initial_table:
    phvwr p.ipsec_int_header_ipsec_cb_index, d.ipsec_cb_index
    phvwr p.ipsec_int_header_payload_start, k.p42p4plus_hdr_ipsec_payload_start
    addi r1, r0, IPV4_HDR_SIZE
    addi r1, r1, ESP_FIXED_HDR_SIZE
    add r2, r1, d.iv_size
    add r2, r2, k.p42p4plus_hdr_ipsec_payload_end
    phvwr p.ipsec_int_header_tailroom_offset, r2
    phvwr p.ipsec_int_header_headroom_offset, k.p42p4plus_hdr_ipsec_payload_start 
    sub r3, k.p42p4plus_hdr_ipsec_payload_end, k.p42p4plus_hdr_ipsec_payload_start
    phvwr p.ipsec_int_header_payload_size, r3
    sub r6, d.block_size, 1
    and r4, r3, r6 
    sub r5, d.block_size, r4
    phvwr p.ipsec_int_header_pad_size, r5
    addi r6, r0, IPSEC_PAD_BYTES_HBM_TABLE_BASE 
    phvwr p.ipsec_to_stage3_pad_addr, r6
    phvwr p.ipsec_to_stage3_pad_size, r5
    add r7, r5, d.icv_size
    addi r7, r7, 2
    phvwr p.ipsec_int_header_tailroom, r7
    phvwr p.ipsec_int_header_l4_protocol, k.p42p4plus_hdr_l4_protocol
    phvwr p.ipsec_global_lif, k.{p4_intr_global_lif_sbit0_ebit2...p4_intr_global_lif_sbit3_ebit10}
    phvwr p.ipsec_global_qtype, k.p4_rxdma_intr_qtype
    phvwr p.ipsec_global_qid, k.p4_rxdma_intr_qid
    phvwr p.ipsec_to_stage3_packet_len, k.p42p4plus_hdr_ipsec_payload_end
    phvwr p.ipsec_to_stage3_iv_size, d.iv_size
    phvwr p.ipsec_to_stage3_iv_salt, d.iv_salt

    phvwr p.ipsec_global_ipsec_cb_addr, k.{p4_rxdma_intr_qstate_addr_sbit0_ebit1...p4_rxdma_intr_qstate_addr_sbit2_ebit33}
 
    phvwri p.app_header_table0_valid, 1
    addi r2, r0, esp_ipv4_tunnel_h2n_allocate_input_desc_semaphore
    srl r2, r2, 6
    phvwr p.common_te0_phv_table_pc, r2 
    phvwri p.common_te0_phv_table_raw_table_size, 3
    phvwri p.common_te0_phv_table_lock_en, 0
    phvwri p.common_te0_phv_table_addr, INDESC_SEMAPHORE_ADDR

    phvwri p.app_header_table1_valid, 1
    addi r2, r0, esp_ipv4_tunnel_h2n_allocate_output_desc_semaphore
    srl r2, r2, 6 
    phvwr p.common_te1_phv_table_pc, r2 
    phvwri p.common_te1_phv_table_raw_table_size, 3
    phvwri p.common_te1_phv_table_lock_en, 0
    phvwri p.common_te1_phv_table_addr, OUTDESC_SEMAPHORE_ADDR
  
    phvwri p.app_header_table2_valid, 1
    addi r2, r0, esp_ipv4_tunnel_h2n_allocate_input_page_semaphore 
    srl r2, r2, 6 
    phvwr p.common_te2_phv_table_pc, r2 
    phvwri p.common_te2_phv_table_raw_table_size, 3
    phvwri p.common_te2_phv_table_lock_en, 0
    phvwri p.common_te2_phv_table_addr, INPAGE_SEMAPHORE_ADDR

    phvwri p.app_header_table3_valid, 1
    addi r2, r0, esp_ipv4_tunnel_h2n_allocate_output_page_semaphore 
    srl r2, r2, 6 
    phvwr p.common_te3_phv_table_pc, r2 
    phvwri p.common_te3_phv_table_raw_table_size, 3
    phvwri p.common_te3_phv_table_lock_en, 0
    phvwri p.common_te3_phv_table_addr, OUTPAGE_SEMAPHORE_ADDR
    nop.e
    nop
 
    

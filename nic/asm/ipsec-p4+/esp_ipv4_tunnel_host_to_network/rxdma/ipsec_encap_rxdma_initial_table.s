#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_defines.h"

struct common_p4plus_stage0_app_header_table_ipsec_encap_rxdma_initial_table_d d;
struct common_p4plus_stage0_app_header_table_k k;
struct phv_ p;

%%
        .param          allocate_input_desc_semaphore
        .param          allocate_output_desc_semaphore
        .param          allocate_input_page_semaphore
        .param          allocate_output_page_semaphore
        .align 
ipsec_encap_rxdma_initial_table:
    phvwr p.ipsec_int_header_ipsec_cb_index, d.ipsec_cb_index
    phvwr p.ipsec_int_header_payload_start, k.p42p4plus_hdr_ipsec_payload_start
    addi r1, r1, ESP_FIXED_HDR_SIZE
    add r2, r1, d.iv_size
    add r2, r2, k.p42p4plus_hdr_ipsec_payload_end
    phvwr p.ipsec_int_header_tailroom_offset, r2
    phvwr p.ipsec_int_header_headroom_offset, r1
    sub r3, k.p42p4plus_hdr_ipsec_payload_end, k.p42p4plus_hdr_ipsec_payload_start
    phvwr p.ipsec_int_header_payload_size, r3
    and r4, r3, d.block_size
    sub r5, d.block_size, r4
    phvwr p.ipsec_int_header_pad_size, r5
    sll r6, r4, 8
    addi r6, r6, IPSEC_PAD_BYTES_TABLE_BASE
    phvwr p.ipsec_to_stage3_pad_addr, r6
    phvwr p.ipsec_to_stage3_pad_size, r5
    add r7, r5, d.icv_size
    addi r7, r7, 2
    phvwr p.ipsec_int_header_tailroom, r7
    phvwr p.ipsec_int_header_l4_protocol, k.p42p4plus_hdr_l4_protocol
    phvwr p.ipsec_global_lif, k.{p4_intr_global_lif_sbit0_ebit2...p4_intr_global_lif_sbit3_ebit10}
    phvwr p.ipsec_global_qtype, k.p4_rxdma_intr_qtype
    phvwr p.ipsec_global_qid, k.p4_rxdma_intr_qid
    //phvwr p.ipsec_global_frame_size, k.{p4_intr_frame_size_sbit0_ebit5...p4_intr_frame_size_sbit6_ebit13}

    phvwri p.p42p4plus_hdr_table0_valid, 1
    addi r2, r0, allocate_input_desc_semaphore
    srl r2, r2, 6
    phvwr p.common_te0_phv_table_pc, r2 
    phvwri p.common_te0_phv_table_raw_table_size, 4
    phvwri p.common_te0_phv_table_lock_en, 0
    phvwri p.common_te0_phv_table_addr, INDESC_SEMAPHORE_ADDR

    phvwri p.p42p4plus_hdr_table1_valid, 1
    addi r2, r0, allocate_output_desc_semaphore
    srl r2, r2, 6 
    phvwr p.common_te1_phv_table_pc, r2 
    phvwri p.common_te1_phv_table_raw_table_size, 4
    phvwri p.common_te1_phv_table_lock_en, 0
    phvwri p.common_te1_phv_table_addr, OUTDESC_SEMAPHORE_ADDR
  
    phvwri p.p42p4plus_hdr_table2_valid, 1
    addi r2, r0, allocate_input_page_semaphore 
    srl r2, r2, 6 
    phvwr p.common_te2_phv_table_pc, r2 
    phvwri p.common_te2_phv_table_raw_table_size, 4
    phvwri p.common_te2_phv_table_lock_en, 0
    phvwri p.common_te2_phv_table_addr, INPAGE_SEMAPHORE_ADDR

    phvwri p.p42p4plus_hdr_table3_valid, 1
    addi r2, r0, allocate_output_page_semaphore 
    srl r2, r2, 6 
    phvwr p.common_te3_phv_table_pc, r2 
    phvwri p.common_te3_phv_table_raw_table_size, 4
    phvwri p.common_te3_phv_table_lock_en, 0
    phvwri p.common_te3_phv_table_addr, OUTPAGE_SEMAPHORE_ADDR
    nop.e
 
    

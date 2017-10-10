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
esp_ipv4_tunnel_n2h_rxdma_initial_table:
    phvwr p.ipsec_int_header_ipsec_cb_index, d.ipsec_cb_index

    phvwri p.p4_intr_global_tm_oport, TM_OPORT_P4INGRESS
    phvwri p.p4_intr_global_tm_iport, TM_OPORT_DMA
    phvwri p.p4_intr_global_lif, 1003

   //payload_start sent by p4 as outer-IP+base-esp(8 bytes)
    //add r1, r0, d.iv_size
    phvwr p.ipsec_int_header_payload_start, k.p42p4plus_hdr_ipsec_payload_start 

    // p4 sends payload_end as end of the packet including 2+icv
    add r2, r0, k.p42p4plus_hdr_ipsec_payload_end
    sub r2, r2, d.icv_size
    subi r2, r2, 2
    phvwr p.ipsec_int_header_tailroom_offset, r2

   //Ethernet header excluding IP, ESP, IV
    //add r3, r0, k.p42p4plus_hdr_ip_hdr_size
    //addi r3, r3, ESP_FIXED_HDR_SIZE 
    //sub r4, k.p42p4plus_hdr_ipsec_payload_start, r3 
    phvwr p.ipsec_int_header_headroom, k.p42p4plus_hdr_ipsec_payload_start 
    add r3, r0, k.p42p4plus_hdr_ipsec_payload_start
    add r3, r3, k.p42p4plus_hdr_ip_hdr_size
    phvwr p.ipsec_int_header_headroom_offset, r3 
  
    add r1, r0, k.p42p4plus_hdr_ipsec_payload_start
    add r1, r1, k.p42p4plus_hdr_ip_hdr_size
    addi r1, r1, ESP_FIXED_HDR_SIZE
    add r1, r1, d.iv_size 
    sub r5, r2, r1
    phvwr p.ipsec_int_header_payload_size, r5
    phvwr p.ipsec_to_stage3_payload_size, r5 

    phvwr p.ipsec_global_lif, k.{p4_intr_global_lif_sbit0_ebit2...p4_intr_global_lif_sbit3_ebit10}
    phvwr p.ipsec_global_qtype, k.p4_rxdma_intr_qtype
    phvwr p.ipsec_global_qid, k.p4_rxdma_intr_qid
    phvwr p.ipsec_global_packet_length, k.p42p4plus_hdr_ipsec_payload_end 
     
    add r6, r0, k.p42p4plus_hdr_ipsec_payload_start
    add r6, r6, k.p42p4plus_hdr_ip_hdr_size
    addi r6, r6, 4
    phvwr p.ipsec_to_stage3_iv_salt_off, r6
    phvwr p.ipsec_to_stage3_iv_salt, d.iv_salt 
    phvwr p.ipsec_to_stage3_ipsec_cb_addr, k.{p4_rxdma_intr_qstate_addr_sbit0_ebit1...p4_rxdma_intr_qstate_addr_sbit2_ebit33}

    // seq-no logic
    slt c1, k.p42p4plus_hdr_seq_no, d.expected_seq_no
    bcf [c1], ipsec_esp_v4_tunnel_n2h_exp_seqno_gt_pak_seqno
    sub r1, d.expected_seq_no, k.p42p4plus_hdr_seq_no
    addi r2, r0, IPSEC_WIN_REPLAY_MAX_DIFF
    slt c2, r2, r1 
    bcf [c2], ipsec_esp_v4_tunnel_n2h_exp_seqno_lt_pak_seqno_diff_more_than_max_allowed
    nop
    addi r7, r0, 1
    sllv r3, r7, r1
    or r3, r3, d.replay_seq_no_bmp 
    // should I do a flush here for this tablewrite as I want next packet to see this updated seqno ??
    tblwr  d.replay_seq_no_bmp, r3
    nop

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
 
ipsec_esp_v4_tunnel_n2h_exp_seqno_lt_pak_seqno_diff_more_than_max_allowed:
    //ori r4, k.ipsec_int_drop_mask, IPSEC_BAD_SEQ_NO
    //phvwr.e p.ipsec_int_drop_mask, r4
    nop.e
    nop

ipsec_esp_v4_tunnel_n2h_exp_seqno_gt_pak_seqno:
    // diff = phv_rx->p42p4plus_hdr.seq_no - phv_rx->ipsec_int.expected_seq_no;
    sub r1, k.p42p4plus_hdr_seq_no, d.expected_seq_no
    slt c1, r1, IPSEC_WIN_REPLAY_MAX_DIFF 
    //if (diff >= ipsec_cb->replay_win_sz) { //can't handle
    bcf [c1], ipsec_esp_v4_tunnel_n2h_exp_seqno_gt_pak_seqno_diff_gt_win_sz
    nop
    // ipsec_cb->replay_win_bmp = (ipsec_cb->replay_win_bmp << diff) | 1;
    sllv r3, d.replay_seq_no_bmp, r1
    ori r3, r3, 1
    tblwr d.replay_seq_no_bmp, r3
    nop
    tblwr d.expected_seq_no, k.p42p4plus_hdr_seq_no
    nop.e
    nop

ipsec_esp_v4_tunnel_n2h_exp_seqno_gt_pak_seqno_diff_gt_win_sz:
    // ipsec_cb->replay_win_bmp = 1;
    //  phv_rx->ipsec_int.drop_mask |= IPSEC_BAD_SEQ_NO;
    addi r7, r0, 1
    tblwr d.replay_seq_no_bmp, r7 
    //ori r4, k.ipsec_int_drop_mask, IPSEC_BAD_SEQ_NO
    //phvwr p.ipsec_int_drop_mask, r4
    nop
    tblwr d.expected_seq_no, k.p42p4plus_hdr_seq_no
    nop.e
    nop
 

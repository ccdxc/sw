#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_defines.h"

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
    phvwr p.ipsec_to_stage3_tail_desc_addr, d.tail_desc_addr

   //payload_start sent by p4 as outer-IP+base-esp(8 bytes)
    add r1, r0, d.iv_size
    phvwr p.ipsec_int_header_payload_start, r1 

    // p4 sends payload_end as end of the packet including 2+icv
    add r2, r0, k.p42p4plus_hdr_ipsec_payload_end
    sub r2, r2, d.icv_size
    subi r2, r2, 2
    phvwr p.ipsec_int_header_tailroom_offset, r2

    add r3, r0, k.p42p4plus_hdr_ip_hdr_size
    addi r3, r3, 8
    sub r4, k.p42p4plus_hdr_ipsec_payload_start, r3 
    phvwr p.ipsec_int_header_headroom, r4 
    phvwr p.ipsec_int_header_headroom_offset, r4 
   
    sub r5, r2, r1
    phvwr p.ipsec_int_header_payload_size, r5
 

    phvwr p.ipsec_global_lif, k.{p4_intr_global_lif_sbit0_ebit2...p4_intr_global_lif_sbit3_ebit10}
    phvwr p.ipsec_global_qtype, k.p4_rxdma_intr_qtype
    phvwr p.ipsec_global_qid, k.p4_rxdma_intr_qid
    //phvwr p.ipsec_global_frame_size, k.{p4_intr_frame_size_sbit0_ebit5...p4_intr_frame_size_sbit6_ebit13}

    // seq-no logic
    slt c1, k.p42p4plus_hdr_seqNo, d.expected_seq_no
    bcf [c1], ipsec_esp_v4_tunnel_n2h_exp_seqno_gt_pak_seqno
    sub r1, d.expected_seq_no, k.p42p4plus_hdr_seqNo
    addi r2, r0, IPSEC_WIN_REPLAY_MAX_DIFF
    slt c2, r1, r2 
    bcf [c2], ipsec_esp_v4_tunnel_n2h_exp_seqno_lt_pak_seqno_diff_more_than_max_allowed
    nop
    addi r7, r0, 1
    sllv r3, r7, r1
    or r3, r3, d.replay_seq_no_bmp 
    // should I do a flush here for this tablewrite as I want next packet to see this updated seqno ??
    tblwr  d.replay_seq_no_bmp, r3
    nop

    phvwri p.p42p4plus_hdr_table0_valid, 1
    addi r2, r0, esp_ipv4_tunnel_n2h_allocate_input_desc_semaphore
    srl r2, r2, 6
    phvwr p.common_te0_phv_table_pc, r2 
    phvwri p.common_te0_phv_table_raw_table_size, 4
    phvwri p.common_te0_phv_table_lock_en, 0
    phvwri p.common_te0_phv_table_addr, INDESC_SEMAPHORE_ADDR

    phvwri p.p42p4plus_hdr_table1_valid, 1
    addi r2, r0, esp_ipv4_tunnel_n2h_allocate_output_desc_semaphore 
    srl r2, r2, 6
    phvwr p.common_te1_phv_table_pc, r2 
    phvwri p.common_te1_phv_table_raw_table_size, 4
    phvwri p.common_te1_phv_table_lock_en, 0
    phvwri p.common_te1_phv_table_addr, OUTDESC_SEMAPHORE_ADDR
  
    phvwri p.p42p4plus_hdr_table2_valid, 1
    addi r2, r0, esp_ipv4_tunnel_n2h_allocate_input_page_semaphore 
    srl r2, r2, 6
    phvwr p.common_te2_phv_table_pc, r2 
    phvwri p.common_te2_phv_table_raw_table_size, 4
    phvwri p.common_te2_phv_table_lock_en, 0
    phvwri p.common_te2_phv_table_addr, INPAGE_SEMAPHORE_ADDR

    phvwri p.p42p4plus_hdr_table3_valid, 1
    addi r2, r0, esp_ipv4_tunnel_n2h_allocate_output_page_semaphore 
    srl r2, r2, 6
    phvwr p.common_te3_phv_table_pc, r2 
    phvwri p.common_te3_phv_table_raw_table_size, 4
    phvwri p.common_te3_phv_table_lock_en, 0
    phvwri p.common_te3_phv_table_addr, OUTPAGE_SEMAPHORE_ADDR
    nop.e
 
ipsec_esp_v4_tunnel_n2h_exp_seqno_lt_pak_seqno_diff_more_than_max_allowed:
    //ori r4, k.ipsec_int_drop_mask, IPSEC_BAD_SEQ_NO
    //phvwr.e p.ipsec_int_drop_mask, r4
    nop

ipsec_esp_v4_tunnel_n2h_exp_seqno_gt_pak_seqno:
    // diff = phv_rx->p42p4plus_hdr.seq_no - phv_rx->ipsec_int.expected_seq_no;
    sub r1, k.p42p4plus_hdr_seqNo, d.expected_seq_no
    slt c1, r1, IPSEC_WIN_REPLAY_MAX_DIFF 
    //if (diff >= ipsec_cb->replay_win_sz) { //can't handle
    bcf [c1], ipsec_esp_v4_tunnel_n2h_exp_seqno_gt_pak_seqno_diff_gt_win_sz
    nop
    // ipsec_cb->replay_win_bmp = (ipsec_cb->replay_win_bmp << diff) | 1;
    sllv r3, d.replay_seq_no_bmp, r1
    ori r3, r3, 1
    tblwr d.replay_seq_no_bmp, r3
    nop

ipsec_esp_v4_tunnel_n2h_exp_seqno_gt_pak_seqno_diff_gt_win_sz:
    // ipsec_cb->replay_win_bmp = 1;
    //  phv_rx->ipsec_int.drop_mask |= IPSEC_BAD_SEQ_NO;
    addi r7, r0, 1
    tblwr d.replay_seq_no_bmp, r7 
    //ori r4, k.ipsec_int_drop_mask, IPSEC_BAD_SEQ_NO
    //phvwr p.ipsec_int_drop_mask, r4
    nop
 

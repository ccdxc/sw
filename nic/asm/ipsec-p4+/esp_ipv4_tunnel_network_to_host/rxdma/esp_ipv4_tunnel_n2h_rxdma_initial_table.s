#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct common_p4plus_stage0_app_header_table_esp_v4_tunnel_n2h_rxdma_initial_table_d d;
struct common_p4plus_stage0_app_header_table_k k;
struct phv_ p;

%%
.param ipsec_esp_v4_tunnel_n2h_good_pkt 
.align
esp_ipv4_tunnel_n2h_rxdma_initial_table:
    phvwr p.ipsec_int_header_ipsec_cb_index, d.ipsec_cb_index
    seq c1, d.is_v6, 1
    phvwri p.p4_intr_global_tm_oport, TM_OPORT_P4INGRESS
    phvwri p.p4_intr_global_tm_iport, TM_OPORT_DMA
    phvwri p.p4_intr_global_lif, ARM_CPU_LIF 

    phvwr p.ipsec_int_header_spi, k.{p42p4plus_hdr_spi_sbit0_ebit15...p42p4plus_hdr_spi_sbit16_ebit31}
    //payload_start sent by p4 as outer-IP+base-esp(8 bytes)
    phvwr p.ipsec_int_header_payload_start, k.{p42p4plus_hdr_ipsec_payload_start_sbit0_ebit7, p42p4plus_hdr_ipsec_payload_start_sbit8_ebit15}
    // p4 sends payload_end as end of the packet including 2+icv
    add r2, r0, k.p42p4plus_hdr_ipsec_payload_end
    addi.c1 r2, r2, IPV6_HDR_SIZE
    sub r2, r2, d.icv_size
    addi r2, r2, IPSEC_SALT_HEADROOM
    phvwr p.ipsec_int_header_tailroom_offset, r2
    phvwr p.ipsec_global_icv_size, d.icv_size

    //Ethernet header excluding IP, ESP, IV
    phvwr p.ipsec_int_header_headroom, k.{p42p4plus_hdr_ipsec_payload_start_sbit0_ebit7, p42p4plus_hdr_ipsec_payload_start_sbit8_ebit15}
    add r3, k.{p42p4plus_hdr_ipsec_payload_start_sbit0_ebit7, p42p4plus_hdr_ipsec_payload_start_sbit8_ebit15}, k.p42p4plus_hdr_ip_hdr_size
    phvwr p.ipsec_int_header_headroom_offset, r3 
  
    add r1, k.{p42p4plus_hdr_ipsec_payload_start_sbit0_ebit7, p42p4plus_hdr_ipsec_payload_start_sbit8_ebit15}, k.p42p4plus_hdr_ip_hdr_size
    addi r1, r1, ESP_FIXED_HDR_SIZE
    add r1, r1, d.iv_size
    subi r2, r2, IPSEC_SALT_HEADROOM
    sub r5, r2, r1
    phvwr p.ipsec_int_header_payload_size, r5
    phvwr p.ipsec_to_stage3_payload_size, r5 
    phvwr p.ipsec_to_stage3_iv_size, d.iv_size

    phvwr p.ipsec_global_lif, k.{p4_intr_global_lif_sbit0_ebit2...p4_intr_global_lif_sbit3_ebit10}
    phvwr p.ipsec_global_qtype, k.p4_rxdma_intr_qtype
    phvwr p.ipsec_global_qid, k.p4_rxdma_intr_qid
    add r2, r0, k.p42p4plus_hdr_ipsec_payload_end 
    addi.c1 r2, r2, IPV6_HDR_SIZE
    phvwr p.ipsec_global_packet_length, r2 
     
    add r6, k.{p42p4plus_hdr_ipsec_payload_start_sbit0_ebit7, p42p4plus_hdr_ipsec_payload_start_sbit8_ebit15}, k.p42p4plus_hdr_ip_hdr_size
    addi r6, r6, ESP_FIXED_HDR_SIZE 
    phvwr p.ipsec_to_stage3_iv_salt_off, r6
    //phvwr p.ipsec_to_stage3_iv_salt, d.iv_salt 
    phvwr p.ipsec_to_stage3_ipsec_cb_addr, k.{p4_rxdma_intr_qstate_addr_sbit0_ebit1...p4_rxdma_intr_qstate_addr_sbit2_ebit33}
    phvwr p.ipsec_to_stage2_ipsec_cb_addr, k.{p4_rxdma_intr_qstate_addr_sbit0_ebit1...p4_rxdma_intr_qstate_addr_sbit2_ebit33}

    // seq-no logic
    seq c4, k.{p42p4plus_hdr_seq_no_sbit0_ebit7, p42p4plus_hdr_seq_no_sbit8_ebit31}, d.expected_seq_no
    bcf [c4], ipsec_esp_v4_tunnel_n2h_exp_seqno_eq_pak_seq_no
    slt c1, k.{p42p4plus_hdr_seq_no_sbit0_ebit7, p42p4plus_hdr_seq_no_sbit8_ebit31}, d.expected_seq_no
    bcf [c1], ipsec_esp_v4_tunnel_n2h_exp_seqno_gt_pak_seqno
    sub r1, k.{p42p4plus_hdr_seq_no_sbit0_ebit7, p42p4plus_hdr_seq_no_sbit8_ebit31}, d.expected_seq_no
    slt c2, IPSEC_WIN_REPLAY_MAX_DIFF, r1 
    bcf [c2], ipsec_esp_v4_tunnel_n2h_exp_seqno_lt_pak_seqno_diff_more_than_max_allowed
    nop
    sllv r3, 1, r1
    or r3, r3, d.replay_seq_no_bmp 
    tblwr  d.replay_seq_no_bmp, r3
    nop
    j ipsec_esp_v4_tunnel_n2h_good_pkt
    nop

.align 
ipsec_esp_v4_tunnel_n2h_exp_seqno_eq_pak_seq_no:
    phvwr p.t2_s2s_last_replay_seq_no, k.{p42p4plus_hdr_seq_no_sbit0_ebit7, p42p4plus_hdr_seq_no_sbit8_ebit31} 
    addi r1, r1, 1
    tblwr d.expected_seq_no, r1 
    nop
    sllv r3, d.replay_seq_no_bmp, 1
    tblwr d.replay_seq_no_bmp, r3
    nop
    j ipsec_esp_v4_tunnel_n2h_good_pkt
    nop

.align 
ipsec_esp_v4_tunnel_n2h_exp_seqno_lt_pak_seqno_diff_more_than_max_allowed:
    sub r3, IPSEC_WIN_REPLAY_MAX_DIFF, r1
    add r4, d.expected_seq_no, r3
    tblwr d.expected_seq_no, r4
    nop
    phvwr p.t2_s2s_last_replay_seq_no,  k.{p42p4plus_hdr_seq_no_sbit0_ebit7, p42p4plus_hdr_seq_no_sbit8_ebit31} 
    sllv r6, d.replay_seq_no_bmp, r3
    sllv r7, 1, r3
    or r6, r6, r7
    tblwr d.replay_seq_no_bmp, r6
    nop
    j ipsec_esp_v4_tunnel_n2h_good_pkt
    nop
    nop.e
    nop
 

.align 
ipsec_esp_v4_tunnel_n2h_exp_seqno_gt_pak_seqno:
    sub r1, d.expected_seq_no, k.{p42p4plus_hdr_seq_no_sbit0_ebit7, p42p4plus_hdr_seq_no_sbit8_ebit31}
    slt c1, r1, IPSEC_WIN_REPLAY_MAX_DIFF 
    bcf [c1], ipsec_esp_v4_tunnel_n2h_exp_seqno_gt_pak_seqno_diff_gt_win_sz
    nop
    sllv r3, d.replay_seq_no_bmp, r1
    ori r3, r3, 1
    tblwr d.replay_seq_no_bmp, r3
    nop
    add r1, k.{p42p4plus_hdr_seq_no_sbit0_ebit7, p42p4plus_hdr_seq_no_sbit8_ebit31}, 1
    tblwr d.expected_seq_no, r1 
    nop
    j ipsec_esp_v4_tunnel_n2h_good_pkt
    nop
    nop.e
    nop

.align 
ipsec_esp_v4_tunnel_n2h_exp_seqno_gt_pak_seqno_diff_gt_win_sz:
    tblwr d.replay_seq_no_bmp, 1 
    nop
    add r1, k.{p42p4plus_hdr_seq_no_sbit0_ebit7, p42p4plus_hdr_seq_no_sbit8_ebit31}, 1
    tblwr d.expected_seq_no, r1 
    nop
    j ipsec_esp_v4_tunnel_n2h_good_pkt
    nop
    nop.e
    nop
 

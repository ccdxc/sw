#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "INGRESS_common_p4plus_stage0_app_header_table_k.h"

struct common_p4plus_stage0_app_header_table_esp_v4_tunnel_n2h_rxdma_initial_table_d d;
struct common_p4plus_stage0_app_header_table_k_ k;
struct phv_ p;

%%
    .align
    .param IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    .param esp_ipv4_tunnel_n2h_allocate_input_desc_semaphore
    .param IPSEC_DEC_NMDR_PI
esp_ipv4_tunnel_n2h_rxdma_initial_table:
    //seq c6, d.is_v6, 0xFF
    //bcf [c6], rxdma_freeze2
    //nop

    add r1, d.cb_pindex, 1
    andi r1, r1, IPSEC_CB_RING_INDEX_MASK
    seq c5, d.cb_cindex, r1

    bcf [c5], esp_ipv4_tunnel_n2h_rxdma_initial_table_cb_ring_full
    nop
    seq c1, d.is_v6, 1
    phvwr p.ipsec_global_cb_pindex, d.cb_pindex
    tblwr d.cb_pindex, r1

    phvwr p.ipsec_int_header_spi, k.p42p4plus_hdr_spi
    //payload_start sent by p4 as outer-IP+base-esp(8 bytes)
    phvwr p.ipsec_int_header_payload_start, k.p42p4plus_hdr_ipsec_payload_start
    // p4 sends payload_end as end of the packet including 2+icv
    add r2, r0, k.p42p4plus_hdr_ipsec_payload_end
    addi.c1 r2, r2, IPV6_HDR_SIZE
    sub r2, r2, IPSEC_DEFAULT_ICV_SIZE
    addi r2, r2, IPSEC_SALT_HEADROOM
    phvwr p.ipsec_int_header_tailroom_offset, r2

    //Ethernet header excluding IP, ESP, IV
    phvwr p.ipsec_int_header_headroom, k.p42p4plus_hdr_ipsec_payload_start
    add r3, k.p42p4plus_hdr_ipsec_payload_start, k.p42p4plus_hdr_ip_hdr_size
    phvwr p.ipsec_int_header_headroom_offset, r3
 
    addi r1, r3, ESP_FIXED_HDR_SIZE
    add r1, r1, d.iv_size
    subi r2, r2, IPSEC_SALT_HEADROOM
    sub r5, r2, r1
    phvwr p.ipsec_int_header_payload_size, r5
    phvwr p.ipsec_to_stage3_payload_size, r5

    add r2, r0, k.p42p4plus_hdr_ipsec_payload_end
    addi.c1 r2, r2, IPV6_HDR_SIZE
    phvwr p.ipsec_global_packet_length, r2
    addi r6, r3, ESP_FIXED_HDR_SIZE
    phvwr p.ipsec_to_stage3_iv_salt_off, r6
    phvwr p.ipsec_to_stage3_ipsec_cb_addr, k.p4_rxdma_intr_qstate_addr
    phvwr p.ipsec_to_stage2_ipsec_cb_addr, k.p4_rxdma_intr_qstate_addr
    //addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    //CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_RXDMA_ENTER_OFFSET, 1)
    addui r1, r0, hiword(IPSEC_DEC_NMDR_PI)
    addi r1, r1, loword(IPSEC_DEC_NMDR_PI)
    CAPRI_CLEAR_TABLE_VALID(1)
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, esp_ipv4_tunnel_n2h_allocate_input_desc_semaphore, r1, TABLE_SIZE_64_BITS)
    phvwr p.ipsec_int_header_ipsec_cb_index, d.ipsec_cb_index
    phvwr p.ipsec_global_ipsec_cb_index, d.ipsec_cb_index
    phvwr p.ipsec_to_stage3_iv_size, d.iv_size
    // seq-no logic
    seq c4, k.p42p4plus_hdr_seq_no, d.expected_seq_no
    bcf [c4], ipsec_esp_v4_tunnel_n2h_exp_seqno_eq_pak_seq_no
    slt c1, k.p42p4plus_hdr_seq_no, d.expected_seq_no
    bcf [c1], ipsec_esp_v4_tunnel_n2h_exp_seqno_gt_pak_seqno
    sub r1, k.p42p4plus_hdr_seq_no, d.expected_seq_no
    slt c2, IPSEC_WIN_REPLAY_MAX_DIFF, r1
    bcf [c2], ipsec_esp_v4_tunnel_n2h_exp_seqno_lt_pak_seqno_diff_more_than_max_allowed
    nop
    sllv r3, 1, r1
    or r3, r3, d.replay_seq_no_bmp
    tblwr.f  d.replay_seq_no_bmp, r3
    nop.e
    nop

.align
ipsec_esp_v4_tunnel_n2h_exp_seqno_eq_pak_seq_no:
    phvwr p.t2_s2s_last_replay_seq_no, k.p42p4plus_hdr_seq_no
    addi r1, r1, 1
    tblwr d.expected_seq_no, r1
    nop
    sllv r3, d.replay_seq_no_bmp, 1
    tblwr d.replay_seq_no_bmp, r3
    nop.e
    nop

.align
ipsec_esp_v4_tunnel_n2h_exp_seqno_lt_pak_seqno_diff_more_than_max_allowed:
    sub r3, IPSEC_WIN_REPLAY_MAX_DIFF, r1
    add r4, d.expected_seq_no, r3
    tblwr d.expected_seq_no, r4
    nop
    phvwr p.t2_s2s_last_replay_seq_no,  k.p42p4plus_hdr_seq_no
    sllv r6, d.replay_seq_no_bmp, r3
    sllv r7, 1, r3
    or r6, r6, r7
    tblwr d.replay_seq_no_bmp, r6
    nop.e
    nop

.align
ipsec_esp_v4_tunnel_n2h_exp_seqno_gt_pak_seqno:
    sub r1, d.expected_seq_no, k.p42p4plus_hdr_seq_no
    slt c1, r1, IPSEC_WIN_REPLAY_MAX_DIFF
    bcf [c1], ipsec_esp_v4_tunnel_n2h_exp_seqno_gt_pak_seqno_diff_gt_win_sz
    nop
    sllv r3, d.replay_seq_no_bmp, r1
    ori r3, r3, 1
    tblwr d.replay_seq_no_bmp, r3
    nop
    add r1, k.p42p4plus_hdr_seq_no, 1
    tblwr d.expected_seq_no, r1
    nop.e
    nop

.align
ipsec_esp_v4_tunnel_n2h_exp_seqno_gt_pak_seqno_diff_gt_win_sz:
    tblwr d.replay_seq_no_bmp, 1
    nop
    add r1, k.p42p4plus_hdr_seq_no, 1
    tblwr d.expected_seq_no, r1
    nop.e
    nop

esp_ipv4_tunnel_n2h_rxdma_initial_table_cb_ring_full:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_RXDMA_CB_RING_FULL_OFFSET, 1)
    phvwri p.p4_intr_global_drop, 1
    nop.e
    nop

rxdma_freeze2:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_RXDMA_FREEZE_OFFSET, 1)
    phvwri p.p4_intr_global_drop, 1
    nop.e
    nop

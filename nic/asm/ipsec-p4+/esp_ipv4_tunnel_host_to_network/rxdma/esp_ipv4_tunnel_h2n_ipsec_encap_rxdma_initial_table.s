#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct common_p4plus_stage0_app_header_table_ipsec_encap_rxdma_initial_table_d d;
struct common_p4plus_stage0_app_header_table_k k;
struct phv_ p;

%%
        .param          IPSEC_PAD_BYTES_HBM_TABLE_BASE
        .align 
esp_ipv4_tunnel_h2n_ipsec_encap_rxdma_initial_table:
    //sub r1, d.{rxdma_ring_cindex}.hx, 1
    //seq c5, d.{rxdma_ring_pindex}.hx, r1
    //bcf [c5], esp_ipv4_tunnel_h2n_ipsec_encap_rxdma_initial_table_cb_ring_full
    add r1, d.cb_pindex, 1
    and r1, r1, IPSEC_CB_RING_INDEX_MASK 
    seq c5, d.cb_cindex, r1
    bcf [c5], esp_ipv4_tunnel_h2n_ipsec_encap_rxdma_initial_table_cb_ring_full
   
    //sub r1, d.{barco_ring_cindex}.hx, 1
    //seq c5, d.{barco_ring_pindex}.hx, r1
    //bcf [c5], esp_ipv4_tunnel_h2n_ipsec_encap_rxdma_initial_table_cb_ring_full
    //add r1, d.{barco_ring_pindex}.hx, 1
    //and r1, r1, IPSEC_CB_RING_INDEX_MASK 
    //seq c5, d.{barco_ring_cindex}.hx, r1
    //bcf [c5], esp_ipv4_tunnel_h2n_ipsec_encap_rxdma_initial_table_cb_ring_full
 
 
    phvwr p.ipsec_global_ipsec_cb_pindex, d.cb_pindex 
    add r7, d.cb_pindex, 1
    andi r7, r7, IPSEC_CB_RING_INDEX_MASK 
    tblwr d.cb_pindex, r7
    phvwrpair p.ipsec_int_header_ipsec_cb_index, d.ipsec_cb_index, p.ipsec_int_header_payload_start, k.{p42p4plus_hdr_ipsec_payload_start_sbit0_ebit7, p42p4plus_hdr_ipsec_payload_start_sbit8_ebit15}
    phvwr p.ipsec_global_ipsec_cb_index, d.ipsec_cb_index
    // I understand that I need to take care of 32 bit overflow into esn-hi etc.
    tbladd d.esn_lo, 1
    tbladd.f d.iv, 1
    smeqb c1, d.flags, IPSEC_FLAGS_V6_MASK, IPSEC_FLAGS_V6_MASK 
    cmov r1, c1, IPV6_HDR_SIZE+ESP_FIXED_HDR_SIZE, IPV4_HDR_SIZE+ESP_FIXED_HDR_SIZE
    add r2, r1, d.iv_size
    add r2, r2, k.p42p4plus_hdr_ipsec_payload_end
    phvwrpair p.ipsec_int_header_headroom_offset, k.{p42p4plus_hdr_ipsec_payload_start_sbit0_ebit7, p42p4plus_hdr_ipsec_payload_start_sbit8_ebit15}, p.ipsec_int_header_tailroom_offset, r2
    sub r3, k.p42p4plus_hdr_ipsec_payload_end, k.{p42p4plus_hdr_ipsec_payload_start_sbit0_ebit7, p42p4plus_hdr_ipsec_payload_start_sbit8_ebit15}
    add.c1 r3, r3, IPV6_HDR_SIZE
    phvwr p.t1_s2s_payload_size, r3
    phvwrpair p.t0_s2s_payload_size, r3, p.t0_s2s_payload_start, k.{p42p4plus_hdr_ipsec_payload_start_sbit0_ebit7, p42p4plus_hdr_ipsec_payload_start_sbit8_ebit15}
    sub r6, d.block_size, 1
    and r4, r3, r6 
    sub r5, d.block_size, r4
    slt c2, r5, 2
    add.c2 r5, r5, d.block_size 
    subi r5, r5, 2
    smeqb c4, d.flags, IPSEC_FLAGS_EXTRA_PAD, IPSEC_FLAGS_EXTRA_PAD
    addi.c4 r5, r5, 64
    phvwrpair p.ipsec_to_stage3_pad_size, r5, p.ipsec_to_stage3_iv_size, d.iv_size
    addui r6, r0, hiword(IPSEC_PAD_BYTES_HBM_TABLE_BASE)
    addi r6, r6, loword(IPSEC_PAD_BYTES_HBM_TABLE_BASE)
    phvwr p.ipsec_to_stage3_pad_addr, r6
    add r7, r5, d.icv_size
    addi r7, r7, 2
    phvwrpair p.ipsec_int_header_tailroom, r7, p.ipsec_int_header_payload_size, r3
    phvwrpair p.ipsec_int_header_pad_size, r5, p.ipsec_int_header_l4_protocol, k.p42p4plus_hdr_l4_protocol
    add  r1, r0, k.p42p4plus_hdr_ipsec_payload_end
    add.c1 r1, r1, IPV6_HDR_SIZE 
    smeqb c3, d.flags, IPSEC_FLAGS_RANDOM_MASK, IPSEC_FLAGS_RANDOM_MASK
    phvwr.c3 p.ipsec_to_stage2_is_random, 1
    phvwr p.ipsec_to_stage3_packet_len, r1 
    phvwr p.ipsec_to_stage3_iv_salt, d.iv_salt
    phvwr p.ipsec_global_ipsec_cb_addr, k.{p4_rxdma_intr_qstate_addr_sbit0_ebit1...p4_rxdma_intr_qstate_addr_sbit2_ebit33}
    phvwrpair p.esp_header_spi, d.spi, p.esp_header_seqno, d.esn_lo
    phvwr.f p.esp_header_iv, d.iv
    nop.e
    nop

esp_ipv4_tunnel_h2n_ipsec_encap_rxdma_initial_table_cb_ring_full:
    phvwri p.p4_intr_global_drop, 1 
    nop.e
    nop

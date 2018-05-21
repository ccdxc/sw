#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_esp_v4_tunnel_n2h_txdma1_initial_table_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_n2h_allocate_barco_req_pindex 
        .param esp_v4_tunnel_n2h_get_in_desc_from_cb_cindex
        .param esp_v4_tunnel_n2h_load_part2
        .param IPSEC_CB_BASE
        .param  TLS_PROXY_BARCO_GCM1_PI_HBM_TABLE_BASE
        .align
esp_ipv4_tunnel_n2h_txdma_initial_table:
    sub r1, d.{barco_ring_cindex}.hx, 1
    seq c5, d.{barco_ring_pindex}.hx, r1
    bcf [c5], esp_ipv4_tunnel_n2h_txdma_initial_table_drop_pkt
    seq c1, d.{rxdma_ring_pindex}.hx, d.{rxdma_ring_cindex}.hx
    b.c1 esp_ipv4_tunnel_n2h_txdma1_initial_table_do_nothing
    phvwri.c1 p.p4_intr_global_drop, 1

    phvwr p.txdma1_global_ipsec_cb_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33} 
    phvwr p.barco_req_command, d.barco_enc_cmd
    phvwr p.t0_s2s_iv_size, d.iv_size
    phvwr p.t0_s2s_icv_size, d.icv_size

    phvwri p.{app_header_table0_valid...app_header_table2_valid}, 7 
    phvwri p.common_te0_phv_table_pc, esp_v4_tunnel_n2h_get_in_desc_from_cb_cindex[33:6] 
    phvwri p.{common_te0_phv_table_lock_en...common_te0_phv_table_raw_table_size}, 11 
    and r2, d.cb_cindex, 0xFF
    sll r2, r2, 3
    add r2, r2, d.cb_ring_base_addr
    add r7, d.cb_cindex, 1
    tblwr d.cb_cindex, r7
    tblmincri.f     d.{rxdma_ring_cindex}.hx, IPSEC_PER_CB_RING_WIDTH, 1
    phvwr p.common_te0_phv_table_addr, r2

    addui       r5, r0, hiword(TLS_PROXY_BARCO_GCM1_PI_HBM_TABLE_BASE)
    addi        r5, r0, loword(TLS_PROXY_BARCO_GCM1_PI_HBM_TABLE_BASE)
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_EN, esp_ipv4_tunnel_n2h_allocate_barco_req_pindex, r5, TABLE_SIZE_16_BITS)

    phvwri p.common_te2_phv_table_pc, esp_v4_tunnel_n2h_load_part2[33:6] 
    phvwri p.{common_te2_phv_table_lock_en...common_te2_phv_table_raw_table_size}, 11 
    add r4, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}, 64
    phvwr p.common_te2_phv_table_addr, r4 

    seq c1, d.{rxdma_ring_pindex}.hx, d.{rxdma_ring_cindex}.hx
    b.!c1 esp_ipv4_tunnel_n2h_txdma1_initial_table_do_nothing 
    addi r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 1, LIF_IPSEC_ESP)
    CAPRI_RING_DOORBELL_DATA(0, d.ipsec_cb_index, 0, 0)
    memwr.dx  r4, r3


esp_ipv4_tunnel_n2h_txdma1_initial_table_do_nothing:
    nop.e
    nop

esp_ipv4_tunnel_n2h_txdma_initial_table_drop_pkt:
    phvwri p.p4_intr_global_drop, 1
    nop.e
    nop
    

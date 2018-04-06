#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_esp_v4_tunnel_n2h_txdma2_initial_table_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_n2h_load_barco_req 
        .param BRQ_BASE
        .align
esp_ipv4_tunnel_n2h_txdma2_initial_table:
    seq c2, d.{barco_ring_pindex}.hx, d.{barco_ring_cindex}.hx
    b.c2 esp_ipv4_tunnel_n2h_txdma2_initial_table_do_nothing
    phvwri.c2 p.p4_intr_global_drop, 1

    seq c1, d.is_v6, 1
    phvwr p.txdma2_global_iv_size, d.iv_size
    phvwr p.txdma2_global_icv_size, d.icv_size

    phvwri p.app_header_table0_valid, 1
    phvwri p.{common_te0_phv_table_lock_en...common_te0_phv_table_raw_table_size}, 14 
    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_n2h_load_barco_req[33:6] 
    and r1, d.barco_cindex, IPSEC_BARCO_RING_INDEX_MASK
    sll r1, r1, BRQ_RING_ENTRY_SIZE_SHIFT 
    add r1, r1, d.barco_ring_base_addr 

    phvwr p.ipsec_to_stage3_ipsec_cb_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}
    phvwr p.ipsec_to_stage3_block_size, d.block_size
    phvwr p.ipsec_to_stage4_vrf_vlan, d.vrf_vlan
    cmov r6, c1, IPV6_ETYPE, IPV4_ETYPE

    add r7, d.barco_cindex, 1
    and r7, r7, IPSEC_BARCO_RING_INDEX_MASK
    tblwr d.barco_cindex, r7
    phvwr  p.common_te0_phv_table_addr, r1
    tblmincri.f  d.{barco_ring_cindex}.hx, IPSEC_BARCO_RING_WIDTH, 1
    phvwr p.ipsec_to_stage4_ip_etype, r6


    seq c2, d.{barco_ring_pindex}.hx, d.{barco_ring_cindex}.hx
    b.!c2 esp_ipv4_tunnel_n2h_txdma2_initial_table_do_nothing
    addi r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 1, LIF_IPSEC_ESP)
    CAPRI_RING_DOORBELL_DATA(0, d.ipsec_cb_index, 1, 0)
    memwr.dx  r4, r3


esp_ipv4_tunnel_n2h_txdma2_initial_table_do_nothing:
    nop.e
    nop


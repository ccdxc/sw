#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_esp_v4_tunnel_n2h_txdma1_initial_table_d d;
struct phv_ p;

%%
        .param esp_v4_tunnel_n2h_txdma1_allocate_barco_req_pindex 
        .param esp_v4_tunnel_n2h_get_in_desc_from_cb_cindex
        .param esp_v4_tunnel_n2h_load_part2
        .param IPSEC_CB_BASE
        .align
esp_ipv4_tunnel_n2h_txdma1_initial_table:
    seq c1, d.{rxdma_ring_pindex}.hx, d.{rxdma_ring_cindex}.hx
    b.c1 esp_ipv4_tunnel_n2h_txdma1_initial_table_do_nothing
    phvwri.c1 p.p4_intr_global_drop, 1

    phvwr p.p4_intr_global_lif, k.{p4_intr_global_lif_sbit0_ebit2...p4_intr_global_lif_sbit3_ebit10}
    phvwr p.p4_intr_global_tm_iq, k.p4_intr_global_tm_iq
    phvwr p.p4_txdma_intr_qtype, k.p4_txdma_intr_qtype
    phvwr p.p4_txdma_intr_qid, k.p4_txdma_intr_qid
    //phvwr p.p4_txdma_intr_qstate_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}
    phvwr p.txdma1_global_ipsec_cb_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33} 
    phvwr p.barco_req_command, d.barco_enc_cmd
    phvwr p.t0_s2s_iv_size, d.iv_size
    phvwr p.t0_s2s_icv_size, d.icv_size

    phvwri p.app_header_table0_valid, 1 
    phvwri p.common_te0_phv_table_lock_en, 1 
    phvwri p.common_te0_phv_table_pc, esp_v4_tunnel_n2h_get_in_desc_from_cb_cindex[33:6] 
    phvwri p.common_te0_phv_table_raw_table_size, 3
    add r1, r0, d.{rxdma_ring_cindex}.hx
    sll r2, r1, 3
    add r2, r2, d.cb_ring_base_addr
    tbladd d.{rxdma_ring_cindex}.hx, 1
    nop
    phvwr p.common_te0_phv_table_addr, r2
    addi r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_CIDX_SET, DB_SCHED_UPD_EVAL, 1, LIF_IPSEC_ESP)
    CAPRI_RING_DOORBELL_DATA(0, d.ipsec_cb_index, 0, d.{rxdma_ring_cindex}.hx)
    memwr.dx  r4, r3
    tbladd d.cb_cindex, 1
 
    addi        r3, r0, CAPRI_BARCO_MD_HENS_REG_GCM0_PRODUCER_IDX
    phvwri p.app_header_table1_valid, 1 
    phvwri p.common_te1_phv_table_lock_en, 1 
    phvwri p.common_te1_phv_table_pc, esp_v4_tunnel_n2h_txdma1_allocate_barco_req_pindex[33:6] 
    phvwri p.common_te1_phv_table_raw_table_size, 2
    phvwr p.common_te1_phv_table_addr, r3 

    phvwri p.app_header_table2_valid, 1 
    phvwri p.common_te2_phv_table_lock_en, 1 
    phvwri p.common_te2_phv_table_pc, esp_v4_tunnel_n2h_load_part2[33:6] 
    phvwri p.common_te2_phv_table_raw_table_size, 3
    add r4, r0, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}
    addi r4, r4, 64
    phvwr p.common_te2_phv_table_addr, r4 

    seq c1, d.{rxdma_ring_pindex}.hx, d.{rxdma_ring_cindex}.hx
    b.!c1 esp_ipv4_tunnel_n2h_txdma1_initial_table_do_nothing
    addi r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_INC, DB_SCHED_UPD_SET, 1, LIF_IPSEC_ESP) 
    phvwr p.barco_req_doorbell_address, r4.dx 
    CAPRI_RING_DOORBELL_DATA(0, d.ipsec_cb_index, 1, 0)
    phvwr p.barco_req_doorbell_data, r3.dx


esp_ipv4_tunnel_n2h_txdma1_initial_table_do_nothing:
    nop.e
    nop
    

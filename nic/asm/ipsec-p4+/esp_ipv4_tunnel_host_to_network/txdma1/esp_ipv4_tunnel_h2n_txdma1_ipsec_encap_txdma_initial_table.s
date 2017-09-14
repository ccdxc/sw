#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_ipsec_encap_txdma_initial_table_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_h2n_txdma1_ipsec_get_in_desc_from_cb_cindex 
        .param esp_ipv4_tunnel_h2n_txdma1_allocate_barco_req_pindex 
        .align
esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_initial_table:
    phvwr p.p4_intr_global_lif, k.{p4_intr_global_lif_sbit0_ebit2...p4_intr_global_lif_sbit3_ebit10}
    phvwr p.p4_intr_global_tm_iq, k.p4_intr_global_tm_iq
    phvwr p.p4_txdma_intr_qtype, k.p4_txdma_intr_qtype
    phvwr p.p4_txdma_intr_qid, k.p4_txdma_intr_qid
    phvwr p.p4_txdma_intr_qstate_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}

    phvwr p.barco_req_brq_barco_enc_cmd, d.barco_enc_cmd
    phvwr p.barco_req_brq_key_index, d.key_index
    
    phvwri p.app_header_table0_valid, 1 
    phvwri p.common_te0_phv_table_lock_en, 1 
    addi r2, r0, esp_ipv4_tunnel_h2n_txdma1_ipsec_get_in_desc_from_cb_cindex 
    srl r2, r2, 6 
    phvwr p.common_te0_phv_table_pc, r2 
    phvwri p.common_te0_phv_table_raw_table_size, 3
    add r1, r0, d.cb_cindex
    sll r2, r1, 3
    add r2, r2, d.cb_ring_base_addr
    phvwr p.common_te0_phv_table_addr, r2
    addi r1, r1, 1
    CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_CIDX_SET, DB_SCHED_UPD_EVAL, 0, LIF_IPSEC_ESP)
    CAPRI_RING_DOORBELL_DATA(0, d.ipsec_cb_index, 0, d.cb_pindex)
    memwr.dx  r4, r3
    tblwr d.cb_cindex, r1
   
    phvwri p.app_header_table1_valid, 1 
    phvwri p.common_te1_phv_table_lock_en, 1 
    addi r2, r0, esp_ipv4_tunnel_h2n_txdma1_allocate_barco_req_pindex 
    srl r2, r2, 6 
    phvwr p.common_te1_phv_table_pc, r2 
    phvwri p.common_te1_phv_table_raw_table_size, 2
    phvwri p.common_te1_phv_table_addr, BRQ_REQ_SEMAPHORE_ADDR
  
    phvwr p.txdma1_global_ipsec_cb_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}
    nop.e
    nop

    




#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_esp_v4_tunnel_n2h_txdma1_initial_table_d d;
struct phv_ p;

%%
        .param esp_v4_tunnel_n2h_txdma1_allocate_barco_req_pindex 
        .param esp_v4_tunnel_n2h_get_in_desc_from_cb_cindex
        .param IPSEC_CB_BASE
        .align
esp_ipv4_tunnel_n2h_txdma1_initial_table:
    phvwr p.p4_intr_global_lif, k.{p4_intr_global_lif_sbit0_ebit2...p4_intr_global_lif_sbit3_ebit10}
    phvwr p.p4_intr_global_tm_iq, k.p4_intr_global_tm_iq
    phvwr p.p4_txdma_intr_qtype, k.p4_txdma_intr_qtype
    phvwr p.p4_txdma_intr_qid, k.p4_txdma_intr_qid
    phvwr p.p4_txdma_intr_qstate_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}

    phvwr p.txdma1_global_ipsec_cb_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33} 
    
    phvwr p.barco_req_command, d.barco_enc_cmd
    phvwr p.barco_req_key_desc_index, d.key_index
    phvwr p.t0_s2s_iv_size, d.iv_size
    phvwr p.t0_s2s_icv_size, d.icv_size

    //Increment PI
    add r1, r0, d.barco_ring_pindex
    addi r1, r1, 1
    phvwr p.barco_dbell_pi, r1.wx   
 
    phvwri p.app_header_table0_valid, 1 
    phvwri p.common_te0_phv_table_lock_en, 1 
    phvwri p.common_te0_phv_table_pc, esp_v4_tunnel_n2h_get_in_desc_from_cb_cindex[33:6] 
    phvwri p.common_te0_phv_table_raw_table_size, 3
    add r1, r0, d.cb_cindex
    sll r2, r1, 3
    add r2, r2, d.cb_ring_base_addr
    phvwr p.common_te0_phv_table_addr, r2
    addi r1, r1, 1
    CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_CIDX_SET, DB_SCHED_UPD_EVAL, 1, LIF_IPSEC_ESP)
    CAPRI_RING_DOORBELL_DATA(0, d.ipsec_cb_index, 0, d.cb_pindex)
    memwr.dx  r4, r3
    tblwr d.cb_cindex, r1
 
    CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_INC, DB_SCHED_UPD_SET, 1, LIF_IPSEC_ESP) 
    phvwr p.barco_req_doorbell_address, r4.dx                                                                                                                                                                                         
    CAPRI_RING_DOORBELL_DATA(0, d.ipsec_cb_index, 1, 0)
    phvwr p.barco_req_doorbell_data, r3.dx

    phvwri p.app_header_table1_valid, 1 
    phvwri p.common_te1_phv_table_lock_en, 1 
    phvwri p.common_te1_phv_table_pc, esp_v4_tunnel_n2h_txdma1_allocate_barco_req_pindex[33:6] 
    phvwri p.common_te1_phv_table_raw_table_size, 3
    phvwri p.common_te1_phv_table_addr, BRQ_REQ_SEMAPHORE_ADDR 
    nop.e
    nop
    

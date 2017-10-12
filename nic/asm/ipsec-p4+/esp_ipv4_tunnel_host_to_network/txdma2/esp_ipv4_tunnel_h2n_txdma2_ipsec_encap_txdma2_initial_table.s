#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_ipsec_encap_txdma2_initial_table_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_barco_req 
        .param BRQ_BASE
        .align
esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_initial_table:
    phvwr p.txdma2_global_ipsec_cb_index, d.ipsec_cb_index
    phvwr p.txdma2_global_iv_size, d.iv_size
    phvwr p.txdma2_global_icv_size, d.icv_size
    phvwri p.p4_intr_global_tm_oport, TM_OPORT_P4INGRESS 
    phvwri p.p4_intr_global_tm_iport, TM_OPORT_DMA 
    phvwri p.p4_intr_global_lif, 1003 
    phvwri p.app_header_table0_valid, 1
    phvwri p.common_te0_phv_table_lock_en, 1
    phvwri p.common_te0_phv_table_raw_table_size, 6
    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_barco_req[33:6] 

    add r1, r0, d.barco_ring_cindex
    sll r1, r1, BRQ_RING_ENTRY_SIZE_SHIFT 
    addi r1, r1, BRQ_BASE 
    phvwr  p.common_te0_phv_table_addr, r1

    phvwr p.ipsec_to_stage3_ipsec_cb_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}

    addi r1, r0, 1
    add r1, r1, d.barco_ring_cindex  
    CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_CIDX_SET, DB_SCHED_UPD_EVAL, 0, LIF_IPSEC_ESP)
    CAPRI_RING_DOORBELL_DATA(0, d.ipsec_cb_index, 1, r1)
    memwr.dx  r4, r3
    tblwr d.barco_ring_cindex, r1

    nop.e
    nop



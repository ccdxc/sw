#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s1_t0_k k;
struct tx_table_s1_t0_ipsec_encap_txdma2_dummy_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_barco_req 
        .align
esp_ipv4_tunnel_h2n_txdma2_ipsec_dummy:
    phvwri p.app_header_table0_valid, 1
    phvwri p.{common_te0_phv_table_lock_en...common_te0_phv_table_raw_table_size}, 6
    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_barco_req[33:6] 
    phvwr  p.common_te0_phv_table_addr, k.ipsec_to_stage1_barco_desc_addr
    phvwri p.{p4_intr_global_tm_iport...p4_intr_global_tm_oport}, ((TM_OPORT_DMA << 4) | TM_OPORT_P4INGRESS) 
    phvwri p.p4_intr_global_lif, ARM_CPU_LIF
    tblmincri.f d.ci, IPSEC_BARCO_RING_WIDTH, 1
    nop.e
    nop

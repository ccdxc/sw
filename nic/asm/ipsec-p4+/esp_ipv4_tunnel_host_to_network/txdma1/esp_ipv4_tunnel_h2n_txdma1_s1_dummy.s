#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s1_t0_k k;
struct tx_table_s1_t0_tx_table_dummy_action_d d;
struct phv_ p;

%%
       .param      TLS_PROXY_BARCO_GCM0_PI_HBM_TABLE_BASE 
       .param      esp_ipv4_tunnel_h2n_txdma1_ipsec_get_in_desc_from_cb_cindex
       .param      esp_ipv4_tunnel_h2n_txdma1_allocate_barco_req_pindex2
       .align
esp_ipv4_tunnel_h2n_txdma1_s1_dummy:
    phvwri p.{app_header_table0_valid...app_header_table1_valid}, 3
    phvwri p.{common_te0_phv_table_lock_en...common_te0_phv_table_raw_table_size}, 5
    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_h2n_txdma1_ipsec_get_in_desc_from_cb_cindex[33:6]
    phvwr  p.common_te0_phv_table_addr, k.ipsec_to_stage1_cb_ring_slot_addr
    addui       r5, r0, hiword(TLS_PROXY_BARCO_GCM0_PI_HBM_TABLE_BASE)
    addi        r5, r0, loword(TLS_PROXY_BARCO_GCM0_PI_HBM_TABLE_BASE)
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_EN, esp_ipv4_tunnel_h2n_txdma1_allocate_barco_req_pindex2, r5, TABLE_SIZE_16_BITS) 
    nop.e
    nop

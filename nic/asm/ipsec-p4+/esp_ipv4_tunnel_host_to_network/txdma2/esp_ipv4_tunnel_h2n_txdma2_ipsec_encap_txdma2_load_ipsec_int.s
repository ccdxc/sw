#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s3_t2_k k;
struct tx_table_s3_t2_ipsec_encap_txdma2_load_ipsec_int_d d;
struct phv_ p;

%%
        .param  esp_ipv4_tunnel_h2n_txdma2_ipsec_update_tx_stats
        .align
esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_ipsec_int:
    phvwri p.{common_te2_phv_table_lock_en...common_te2_phv_table_raw_table_size}, 14
    phvwri p.common_te2_phv_table_pc, esp_ipv4_tunnel_h2n_txdma2_ipsec_update_tx_stats[33:6]
    add r4, k.ipsec_to_stage3_ipsec_cb_addr, IPSEC_H2N_STATS_CB_OFFSET
    phvwr  p.common_te2_phv_table_addr, r4 

    phvwr p.txdma2_global_pad_size, d.pad_size
    phvwr p.txdma2_global_l4_protocol, d.l4_protocol
    phvwr p.txdma2_global_payload_size, d.payload_size
    phvwr.e p.t0_s2s_tailroom_offset, d.tailroom_offset
    nop

// need to add error code
esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_barco_error:
    phvwri p.p4_intr_global_drop, 1
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0
    phvwri.e p.ipsec_to_stage4_barco_error, 1
    nop
 

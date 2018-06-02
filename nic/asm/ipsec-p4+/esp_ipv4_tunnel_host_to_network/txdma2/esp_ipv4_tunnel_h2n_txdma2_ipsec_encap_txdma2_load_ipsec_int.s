#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s3_t2_k k;
struct tx_table_s3_t2_ipsec_encap_txdma2_load_ipsec_int_d d;
struct phv_ p;

%%
        .align
esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_ipsec_int:
    seq c1, d.{status}.dx, 0
    bcf [!c1], esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_barco_error
    phvwri p.app_header_table2_valid, 0
    phvwr p.txdma2_global_pad_size, d.pad_size
    phvwr p.txdma2_global_l4_protocol, d.l4_protocol
    phvwr p.txdma2_global_payload_size, d.payload_size
    phvwr p.t0_s2s_headroom_offset, d.headroom_offset
    phvwr p.t0_s2s_tailroom_offset, d.tailroom_offset
    nop.e
    nop

// need to add error code
esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_barco_error:
    phvwri p.p4_intr_global_drop, 1
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0
    phvwri p.ipsec_to_stage4_barco_error, 1
    nop.e
    nop
 

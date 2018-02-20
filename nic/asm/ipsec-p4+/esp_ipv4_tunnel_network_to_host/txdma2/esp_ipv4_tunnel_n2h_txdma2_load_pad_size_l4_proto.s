#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct tx_table_s3_t0_k k;
struct tx_table_s3_t0_esp_v4_tunnel_n2h_txdma2_load_pad_size_and_l4_proto_d d;
struct phv_ p;

%%
        .align
        .param esp_v4_tunnel_n2h_txdma2_build_decap_packet

esp_ipv4_tunnel_n2h_txdma2_load_pad_size_l4_proto:
    sub r2, k.ipsec_to_stage3_block_size, 1
    and r1, d.pad_size, r2 
    phvwr p.txdma2_global_pad_size, r1 
    phvwri p.app_header_table0_valid, 1
    phvwri p.common_te0_phv_table_lock_en, 1
    phvwri p.common_te0_phv_table_raw_table_size, 6
    phvwri p.common_te0_phv_table_pc, esp_v4_tunnel_n2h_txdma2_build_decap_packet[33:6]
    phvwr  p.common_te0_phv_table_addr, k.ipsec_to_stage3_ipsec_cb_addr
    nop.e
    nop


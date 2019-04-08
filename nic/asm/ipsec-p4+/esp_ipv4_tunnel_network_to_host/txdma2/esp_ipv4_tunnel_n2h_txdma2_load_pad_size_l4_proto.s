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
        .param esp_ipv4_tunnel_n2h_txdma2_ipsec_update_tx_stats

esp_ipv4_tunnel_n2h_txdma2_load_pad_size_l4_proto:
    and r1, d.pad_size, (IPSEC_BLOCK_SIZE - 1)
    phvwr p.txdma2_global_pad_size, r1 
    add r5, r0, k.ipsec_to_stage3_ipsec_cb_addr
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, esp_v4_tunnel_n2h_txdma2_build_decap_packet, r5, TABLE_SIZE_512_BITS)
    addi r3, r5, IPSEC_N2H_STATS_CB_OFFSET
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_EN, esp_ipv4_tunnel_n2h_txdma2_ipsec_update_tx_stats, r3, TABLE_SIZE_512_BITS)
    nop.e
    nop


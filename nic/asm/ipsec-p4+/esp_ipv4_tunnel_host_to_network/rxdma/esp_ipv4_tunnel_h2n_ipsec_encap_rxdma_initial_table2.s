#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct common_p4plus_stage0_app_header_table_ipsec_encap_rxdma_initial_table_d d;
struct common_p4plus_stage0_app_header_table_k k;
struct phv_ p;

%%

esp_ipv4_tunnel_h2n_ipsec_encap_rxdma_initial_table2:
    smeqb c3, d.flags, IPSEC_FLAGS_RANDOM_MASK, IPSEC_FLAGS_RANDOM_MASK
    phvwr.c3 p.ipsec_to_stage2_is_random, 1
    phvwr p.ipsec_global_ipsec_cb_index, d.ipsec_cb_index
    phvwr p.ipsec_to_stage3_iv_salt, d.iv_salt
    phvwrpair p.esp_header_spi, d.spi, p.esp_header_seqno, d.esn_lo
    phvwr.e p.esp_header_iv, d.iv
    nop

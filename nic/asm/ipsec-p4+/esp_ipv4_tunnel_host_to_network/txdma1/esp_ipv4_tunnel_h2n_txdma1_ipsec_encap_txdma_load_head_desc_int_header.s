#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s2_t0_k k;
struct tx_table_s2_t0_ipsec_encap_txdma_load_head_desc_int_header_d d;
struct phv_ p;

%%
        .align
esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_load_head_desc_int_header:
    phvwr p.barco_req_brq_in_addr, d.in_desc
    phvwr p.barco_req_brq_out_addr, d.out_desc
    add r1, r0, d.pad_size
    add r1, r1, d.tailroom_offset
    addi r1, r1, 2
    add r1, r1, d.out_page
    phvwr p.barco_req_brq_auth_tag_addr, r1
    phvwr p.barco_req_brq_hdr_size, d.payload_start


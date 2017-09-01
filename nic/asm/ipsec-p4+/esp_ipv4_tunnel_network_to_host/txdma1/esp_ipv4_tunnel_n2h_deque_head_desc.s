#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct tx_table_s2_t2_k k;
struct tx_table_s2_t2_esp_v4_tunnel_n2h_txdma1_dequeue_head_desc_d d;
struct phv_ p;

%%
        .align
esp_v4_tunnel_n2h_txdma1_dequeue_head_desc:
    phvwr p.t0_s2s_in_page_addr, d.addr0
    phvwri p.app_header_table1_valid, 0
    nop.e

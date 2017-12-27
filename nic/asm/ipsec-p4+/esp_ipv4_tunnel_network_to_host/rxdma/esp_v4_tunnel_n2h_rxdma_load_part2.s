#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct rx_table_s3_t2_k k;
struct rx_table_s3_t2_esp_v4_tunnel_n2h_load_part2_d d;
struct phv_ p;

%%
    .align
esp_v4_tunnel_n2h_rxdma_load_part2:
    phvwri p.app_header_table2_valid, 0
    phvwr p.ipsec_to_stage3_iv_salt, d.iv_salt
    tblwr d.last_replay_seq_no, k.t2_s2s_last_replay_seq_no
    nop.e
    nop
    


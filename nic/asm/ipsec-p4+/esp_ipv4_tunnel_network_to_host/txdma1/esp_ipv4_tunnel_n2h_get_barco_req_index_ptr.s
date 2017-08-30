#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_defines.h"

struct tx_table_s2_t0_k k;
struct tx_table_s2_t0_esp_v4_tunnel_n2h_txdma1_get_barco_req_index_ptr_d d;
struct phv_ p;

%%
        .param esp_v4_tunnel_n2h_write_barco_req 
        .align
esp_v4_tunnel_n2h_txdma1_get_barco_req_index_ptr:
    phvwri p.app_header_table0_valid, 1
    addi r2, r0, esp_v4_tunnel_n2h_write_barco_req
    srl r2, r2, 6
    phvwr p.common_te0_phv_table_pc, r2 
    phvwri p.common_te0_phv_table_lock_en, 1
    phvwri p.common_te0_phv_table_raw_table_size, 6
    phvwr  p.common_te0_phv_table_addr, d.barco_req_index_address
    phvwri p.app_header_table2_valid, 0
    

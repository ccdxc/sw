#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_defines.h"

struct tx_table_s1_t0_k k;
struct tx_table_s1_t0_ipsec_encap_txdma2_load_barco_req_ptr_d d;
struct phv_ p;

%%
        .param ipsec_encap_txdma2_load_barco_req
        .align
ipsec_get_barco_req_index_ptr:
    phvwri p.app_header_table0_valid, 1
    phvwri p.common_te0_phv_table_lock_en, 1
    phvwri p.common_te0_phv_table_raw_table_size, 6 
    phvwri p.common_te0_phv_table_pc, ipsec_encap_txdma2_load_barco_req 
    phvwr  p.common_te0_phv_table_addr, d.barco_req_address 
    nop.e


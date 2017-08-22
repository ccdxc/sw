#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_defines.h"

struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_ipsec_encap_txdma2_initial_table_d d;
struct phv_ p;

%%
        .param ipsec_encap_txdma2_load_barco_req_ptr
        .align
ipsec_encap_txdma2_initial_table:
    phvwr p.txdma2_global_ipsec_cb_index, d.ipsec_cb_index
    phvwr p.txdma2_global_iv_size, d.iv_size
    phvwr p.txdma2_global_icv_size, d.icv_size

    phvwri p.app_header_table0_valid, 1
    phvwri p.common_te0_phv_table_lock_en, 1
    phvwri p.common_te0_phv_table_raw_table_size, 3
    phvwri p.common_te0_phv_table_pc, ipsec_encap_txdma2_load_barco_req_ptr 
    add r1, r0, d.barco_cb_cindex
    sll r1, r1, BRQ_RING_ENTRY_SIZE_SHIFT 
    addi r1, r1, BRQ_REQ_RING_BASE_ADDR
    phvwr  p.common_te0_phv_table_addr, r1



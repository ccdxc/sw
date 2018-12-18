#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s2_t0_ooq_tcp_tx_k.h"

struct phv_ p;
struct s2_t0_ooq_tcp_tx_k_ k;
struct s2_t0_ooq_tcp_tx_ooq_tcp_txdma_load_qbase_addr_d d;

%%
    .align
    .param tcp_ooo_txdma_load_one_descr_addr
tcp_ooo_txdma_load_ooq_ring_base_addr:
    add r1, d.ooq_ring_base_addr, r0
    phvwri p.to_s5_curr_index, 1
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, tcp_ooo_txdma_load_one_descr_addr, r1, TABLE_SIZE_64_BITS) 
    nop.e
    nop

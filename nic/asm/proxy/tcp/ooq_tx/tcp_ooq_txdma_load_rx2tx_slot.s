#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s1_t0_ooq_tcp_tx_k.h"

struct phv_ p;
struct s1_t0_ooq_tcp_tx_k_ k;
struct s1_t0_ooq_tcp_tx_ooq_tcp_txdma_load_rx2tx_slot_d d;

%%
    .align
    .param tcp_ooo_txdma_load_one_descr_addr
tcp_ooq_txdma_load_rx2tx_slot:
    phvwr p.to_s4_new_qbase_addr, d.ooq_qbase_addr
    phvwr p.to_s4_num_entries, d.num_entries
    add r1, d.ooq_qbase_addr, r0
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, tcp_ooo_txdma_load_one_descr_addr, r1, TABLE_SIZE_64_BITS)
    nop.e
    nop

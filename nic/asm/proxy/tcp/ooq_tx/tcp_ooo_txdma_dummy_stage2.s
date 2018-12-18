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
struct s2_t0_ooq_tcp_tx_tx_table_dummy_action_d d;

%%
    .align
    .param tcp_ooo_txdma_dummy_stage3
tcp_ooo_txdma_dummy_stage2:
    CAPRI_NEXT_TABLE0_READ_NO_TABLE_LKUP(tcp_ooo_txdma_dummy_stage3)
    nop.e
    nop

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
struct s1_t0_ooq_tcp_tx_ooq_tcp_txdma_load_current_qbase_descr_addr_d d;

%%
    .align
    .param tcp_ooo_txdma_dummy_stage2
tcp_ooq_txdma_load_curr_descr:
    phvwr p.to_s3_curr_rnmdr_addr, d.one_descr_addr
    CAPRI_NEXT_TABLE0_READ_NO_TABLE_LKUP(tcp_ooo_txdma_dummy_stage2)
    nop.e
    nop

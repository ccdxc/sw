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
tcp_ooo_txdma_load_one_descr_addr:
    add r1, d.ooq_descr_addr, r0
    nop.e
    nop

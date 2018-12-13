#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s4_t0_ooq_tcp_tx_k.h"

struct phv_ p;
struct s4_t0_ooq_tcp_tx_k_ k;
struct s4_t0_ooq_tcp_tx_ooq_tcp_txdma_generate_dummy_pkt_d d;

%%
    .align
tcp_ooo_txdma_load_qstate_and_update:
    nop.e
    nop

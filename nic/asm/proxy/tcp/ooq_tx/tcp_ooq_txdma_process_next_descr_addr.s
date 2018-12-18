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
tcp_ooq_txdma_process_next_descr_addr:
    nop.e
    nop

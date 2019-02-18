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
struct s2_t0_ooq_tcp_tx_set_current_ooq_d d;

%%
    .align
tcp_ooo_txdma_set_current_ooq:
    tblwr           d.curr_ooo_qbase, k.to_s2_qbase_addr
    tblwr           d.curr_ooq_num_entries, k.to_s2_num_entries
    tblwr           d.curr_ooq_trim, k.to_s2_trim
    tblwr.f         d.curr_index, 0

    phvwri          p.p4_intr_global_drop, 1
    CAPRI_CLEAR_TABLE_VALID(0)
    nop.e
    nop

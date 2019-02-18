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
struct s1_t0_ooq_tcp_tx_load_rx2tx_slot_d d;

%%
    .align
    .param          tcp_ooo_txdma_set_current_ooq
tcp_ooq_txdma_load_rx2tx_slot:
    phvwr           p.to_s2_qbase_addr, d.qbase_addr
    phvwr           p.to_s2_num_entries, d.num_entries
    phvwr           p.to_s2_trim, d.trim
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, tcp_ooo_txdma_set_current_ooq, 
                        k.common_phv_qstate_addr, TABLE_SIZE_512_BITS)
    nop.e
    nop

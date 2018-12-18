#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s5_t0_ooq_tcp_tx_k.h"

struct phv_ p;
struct s5_t0_ooq_tcp_tx_k_ k;
struct s5_t0_ooq_tcp_tx_ooq_tcp_txdma_generate_dummy_pkt_d d;

%%
    .align
tcp_ooo_txdma_load_qstate_and_update:
    add r1, k.to_s5_curr_index, 1
    andi r1, r1, TCP_MAX_OOQ_DESCR_MASK
    tblwr d.curr_index, r1
    phvwr p.{p4_intr_global_tm_iport...p4_intr_global_tm_oport}, ( (9 << 4) | 9)
    phvwr p.p4_intr_global_lif, LIF_TCP
    phvwr p.p4_txdma_intr_qtype, 0
    phvwr p.p4_txdma_intr_dma_cmd_ptr, CAPRI_PHV_START_OFFSET(intrinsic_dma_dma_cmd_type)
    phvwr p.p4_txdma_intr_qid, k.common_phv_fid

    seq c1, k.to_s5_new_processing, 1
    tblwr.c1 d.current_descr_qbase_addr, k.to_s5_new_qbase_addr
    tblwr.c1 d.num_entries, k.to_s5_num_entries
    nop.e
    nop

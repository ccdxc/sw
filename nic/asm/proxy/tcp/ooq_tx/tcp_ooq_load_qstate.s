#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s0_t0_ooq_tcp_tx_k.h"
#include "nic/p4/common/defines.h"
#include "tcp-phv.h"

struct phv_ p;
struct s0_t0_ooq_tcp_tx_k_ k;
struct s0_t0_ooq_tcp_tx_load_stage0_d d;

%%
    .align
    .param          tcp_ooq_txdma_load_rx2tx_slot
    .param          tcp_ooq_txdma_process_next_descr_addr
    .param          tcp_ooq_free_queue
    .param          tcp_ooq_txdma_win_upd_phv2pkt_dma

tcp_qtype1_process:
    CAPRI_OPERAND_DEBUG(r7)
    .brbegin
       // priorities are 0 (highest) to 7 (lowest)
       brpri		r7[1:0], [0,1]
       nop
          .brcase 0
             b tcp_ooq_load_qstate            // prio 1
             nop
          .brcase 1
             b tcp_trigger_window_update      // prio 0
             nop
          .brcase 2
             b tcp_ooq_load_qstate_do_nothing // prio 2
             nop
    .brend

tcp_trigger_window_update:
    tblwr.f         d.{ci_1}.hx, d.{pi_1}.hx

    phvwrpair       p.common_phv_fid, k.p4_txdma_intr_qid, \
                        p.common_phv_qstate_addr, k.p4_txdma_intr_qstate_addr
    phvwri          p.{p4_intr_global_tm_iport...p4_intr_global_tm_oport}, (TM_PORT_DMA << 4) | TM_PORT_DMA
    phvwri          p.p4_txdma_intr_dma_cmd_ptr, TCP_PHV_OOQ_TXDMA_COMMANDS_START
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, tcp_ooq_txdma_win_upd_phv2pkt_dma)

    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, TCP_OOO_RX2TX_QTYPE, LIF_TCP)
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA_NOP(k.p4_txdma_intr_qid)

    memwr.dx.e        r4, r3
    nop

tcp_ooq_load_qstate:
    phvwrpair       p.common_phv_fid, k.p4_txdma_intr_qid, \
                        p.common_phv_qstate_addr, k.p4_txdma_intr_qstate_addr
    seq             c1, d.ooq_work_in_progress, 0
    b.c1            tcp_ooq_load_qstate_process_new_request
    nop
    
tcp_ooq_load_qstate_process_next_pkt_descr:
    /*
     * TODO : Figure out a way to trim the first few bytes (d.curr_ooq_trim)
     */
    seq             c1, d.curr_ooo_qbase, 0
    b.c1            tcp_ooq_load_qstate_do_nothing
    add             r1, d.curr_ooo_qbase, d.curr_index, NIC_OOQ_ENTRY_SIZE_SHIFT
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, tcp_ooq_txdma_process_next_descr_addr, r1, TABLE_SIZE_64_BITS)

    // check if we are done with current OOQ
    tbladd          d.curr_index, 1
    seq             c1, d.curr_index, d.curr_ooq_num_entries
    b.!c1           tcp_ooq_skip_doorbell
    nop

    // we are done with current queue, ring doorbell if pi == ci
    tblwr           d.ooq_work_in_progress, 0
    tblmincri.f     d.{ci_0}.hx, ASIC_OOO_RX2TX_RING_SLOTS_SHIFT, 1

    // Inform producer of CI
    add             r1, r0, d.ooo_rx2tx_producer_ci_addr
    memwr.h         r1, d.{ci_0}.hx

    // launch table to free queue
    phvwr           p.to_s1_qbase_addr, d.curr_ooo_qbase
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_EN, tcp_ooq_free_queue, d.ooo_rx2tx_free_pi_addr, TABLE_SIZE_32_BITS)

    seq             c1, d.{ci_0}.hx, d.{pi_0}.hx
    b.!c1           tcp_ooq_skip_doorbell

    /*
     * We are done with all OOO queues
     * Ring doorbell to eval
     */
    phvwr           p.common_phv_all_ooq_done, 1
    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP,
                        DB_SCHED_UPD_EVAL, TCP_OOO_RX2TX_QTYPE, LIF_TCP)
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA_NOP(k.p4_txdma_intr_qid)
    memwr.dx        r4, r3

tcp_ooq_skip_doorbell:
    nop.e
    nop

tcp_ooq_load_qstate_process_new_request:
    // New request
    add             r1, d.ooo_rx2tx_qbase, d.{ci_0}.hx, NIC_OOQ_RX2TX_ENTRY_SIZE_SHIFT
    //launch table with this address
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, tcp_ooq_txdma_load_rx2tx_slot, r1, TABLE_SIZE_64_BITS)
    tblwr           d.curr_ooo_qbase, 0
    tblwr.f         d.ooq_work_in_progress, 1

    wrfence.e
    nop

tcp_ooq_load_qstate_do_nothing:
    phvwri          p.p4_intr_global_drop, 1
    nop.e
    nop


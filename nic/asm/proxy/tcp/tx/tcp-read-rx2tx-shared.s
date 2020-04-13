/*
 *      Implements the rx2tx shared state read stage of the TxDMA P4+ pipeline
 */
#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s0_t0_tcp_tx_k.h"

struct phv_ p;
struct s0_t0_tcp_tx_k_ k;
struct s0_t0_tcp_tx_read_rx2tx_d d;

%%
    .align
    .param          tcp_tx_read_rx2tx_shared_extra_stage1_start
    .param          tcp_tx_read_rx2tx_shared_extra_idle_start
    .param          tcp_tx_read_rx2tx_shared_extra_clean_retx_stage1_start
    .param          tcp_tx_sesq_read_ci_stage1_start
    .param          tcp_tx_sesq_read_retx_ci_stage1_start


// mask ring clean_retx and pending_tx when barrier is set
#define CLEAN_RETX_PENDING_MASK         0x5f

tcp_tx_read_rx2tx_shared_process:
    CAPRI_OPERAND_DEBUG(r7)
    // if barrier is set, mask out clean_retx ring from the list of rings
    // to eval
    seq             c1, d.clean_retx_pending, CLEAN_RETX_PENDING_MASK
    and.c1          r7, r7, d.clean_retx_pending
    .brbegin
        // priorities are 0 (highest) to 7 (lowest)
        // The rightmost value specifies the priority of r7[0]
        // fast timer ring is set to highest priority so we handle timer
        // in all cases (example window full also)
        brpri        r7[7:0], [1,4,2,6,7,0,5,3]
        nop
            .brcase 0
                b tcp_tx_launch_sesq            // prio 3
                nop
            .brcase 1
                b tcp_tx_send_ack               // prio 5
                nop
            .brcase 2
                b tcp_tx_ft_expired             // prio 0
                nop
            .brcase 3
                b tcp_tx_del_ack                // prio 7
                nop
            .brcase 4
                b tcp_tx_launch_asesq           // prio 6
                nop
            .brcase 5
                b tcp_tx_launch_pending_tx      // prio 2
                nop
            .brcase 6
                b tcp_tx_fast_retrans           // prio 4
                nop
            .brcase 7
                b tcp_tx_clean_retx             // prio 1
                nop
            .brcase 8
                b tcp_tx_abort
                nop
    .brend

/******************************************************************************
 * tcp_tx_launch_sesq
 *****************************************************************************/
tcp_tx_launch_sesq:
#ifndef HW
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_DONT_TX, TCP_TX_DDOL_DONT_TX
    phvwri.c1       p.common_phv_debug_dol_dont_tx, 1
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_START_RETX_TIMER, TCP_TX_DDOL_START_RETX_TIMER
    phvwri.!c1      p.common_phv_debug_dol_dont_start_retx_timer, 1
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_FORCE_TBL_SETADDR, TCP_TX_DDOL_FORCE_TBL_SETADDR
    phvwri.c1       p.common_phv_debug_dol_force_tbl_setaddr, 1
#endif
    bbeq            d.sesq_tx_ci[15], 0, tcp_tx_sesq_no_window
    add             r5, d.{sesq_base}.wx, d.{ci_0}.hx, NIC_SESQ_ENTRY_SIZE_SHIFT

    // start perpetual timer if necessary for the flow
    seq             c1, d.perpetual_timer_started, 0
    bal.c1          r7, tcp_tx_start_perpetual_timer
    // store current ci in r6
    add             r6, r0, d.{ci_0}.hx

    tblmincri.f     d.{ci_0}.hx, ASIC_SESQ_RING_SLOTS_SHIFT, 1
    phvwr           p.common_phv_debug_dol_bypass_barco, d.debug_dol_tx[TCP_TX_DDOL_BYPASS_BARCO_BIT]
    phvwr           p.to_s4_sesq_tx_ci, r6

    /*
     * Ring doorbell to set CI if pi == ci
     */
    seq             c1, d.{ci_0}.hx, d.{pi_0}.hx
    b.!c1           tcp_tx_skip_sesq_doorbell

    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    CAPRI_RING_DOORBELL_DATA_NOP(k.p4_txdma_intr_qid)
    memwr.dx        r4, r3

tcp_tx_skip_sesq_doorbell:
    // This table need not be locked since it is read-only.
    // Moreover it should not be locked to prevent the bypass
    // cache from being used (bypass cache cannot be used,
    // since the contents are written from rx pipeline)
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS,
                        tcp_tx_read_rx2tx_shared_extra_stage1_start,
                        k.p4_txdma_intr_qstate_addr,
                        TCP_TCB_RX2TX_SHARED_EXTRA_OFFSET, TABLE_SIZE_512_BITS)
    phvwrpair       p.common_phv_fid, k.p4_txdma_intr_qid, \
                        p.common_phv_qstate_addr, k.p4_txdma_intr_qstate_addr

    phvwri          p.common_phv_pending_sesq, 1
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, tcp_tx_sesq_read_ci_stage1_start,
                     r5, TABLE_SIZE_64_BITS)

tcp_tx_launch_sesq_end:
    // DEBUG ONLY code
#ifndef HW
    add             r1, k.p4_txdma_intr_qstate_addr, TCP_DDOL_TBLADDR_SHIFT_OFFSET
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_FORCE_TBL_SETADDR, TCP_TX_DDOL_FORCE_TBL_SETADDR
    tblsetaddr.c1   r1, 2
    tblwr           d.debug_dol_tblsetaddr, TCP_DDOL_TBLADDR_VALUE
#endif
    // END of DEBUG code
    nop.e
    nop

tcp_tx_sesq_no_window:
    tblwr.f         d.{ci_0}.hx, d.{pi_0}.hx
    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    CAPRI_RING_DOORBELL_DATA_NOP(k.p4_txdma_intr_qid)
    memwr.dx.e      r4, r3
    phvwri          p.p4_intr_global_drop, 1


/******************************************************************************
 * tcp_tx_launch_asesq
 *****************************************************************************/
tcp_tx_launch_asesq:
    // start perpetual timer if necessary for the flow
    seq             c1, d.perpetual_timer_started, 0
    bal.c1          r7, tcp_tx_start_perpetual_timer

    phvwri          p.common_phv_pending_asesq, 1
#ifndef HW
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_DONT_TX, TCP_TX_DDOL_DONT_TX
    phvwri.c1       p.common_phv_debug_dol_dont_tx, 1
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_START_RETX_TIMER, TCP_TX_DDOL_START_RETX_TIMER
    phvwri.!c1      p.common_phv_debug_dol_dont_start_retx_timer, 1
#endif

    // asesq_base = sesq_base - number of sesq slots
    sub             r3, d.{sesq_base}.wx, ASIC_SESQ_RING_SLOTS, NIC_SESQ_ENTRY_SIZE_SHIFT

    and             r1, d.{ci_4}.hx, (ASIC_ASESQ_RING_SLOTS - 1)
    add             r3, r3, r1, NIC_SESQ_ENTRY_SIZE_SHIFT
    tbladd.f        d.{ci_4}.hx, 1
    phvwr           p.common_phv_debug_dol_bypass_barco, d.debug_dol_tx[TCP_TX_DDOL_BYPASS_BARCO_BIT]

    // This table need not be locked since it is read-only.
    // Moreover it should not be locked to prevent the bypass
    // cache from being used (bypass cache cannot be used,
    // since the contents are written from rx pipeline)
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS,
                        tcp_tx_read_rx2tx_shared_extra_stage1_start,
                        k.p4_txdma_intr_qstate_addr,
                        TCP_TCB_RX2TX_SHARED_EXTRA_OFFSET, TABLE_SIZE_512_BITS)
    phvwrpair       p.common_phv_fid, k.p4_txdma_intr_qid, \
                        p.common_phv_qstate_addr, k.p4_txdma_intr_qstate_addr
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, tcp_tx_sesq_read_ci_stage1_start,
                     r3, TABLE_SIZE_64_BITS)

    /*
     * Ring doorbell to set CI if pi == ci
     */
    seq             c1, d.{ci_4}.hx, d.{pi_4}.hx
    b.!c1           tcp_tx_launch_asesq_end

    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA_NOP(k.p4_txdma_intr_qid)
    memwr.dx        r4, r3

tcp_tx_launch_asesq_end:
    nop.e
    nop


/******************************************************************************
 * tcp_tx_launch_pending_tx
 *****************************************************************************/
tcp_tx_launch_pending_tx:
    // if sesq_tx_ci is invalid, quit
    bbeq            d.sesq_tx_ci[15], 1, tcp_tx_launch_pending_abort

    phvwr           p.to_s4_window_open, 1
    tblwr           d.{ci_0}.hx, d.sesq_tx_ci
    tblwr           d.sesq_tx_ci, TCP_TX_INVALID_SESQ_TX_CI
    b               tcp_tx_launch_sesq
 tcp_tx_launch_pending_abort:
    tblwr           d.{ci_5}.hx, d.{pi_5}.hx

    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA_NOP(k.p4_txdma_intr_qid)
    phvwri.e        p.p4_intr_global_drop, 1
    memwr.dx        r4, r3

/******************************************************************************
 * tcp_tx_send_ack
 *****************************************************************************/
tcp_tx_send_ack:
    // start perpetual timer if necessary for the flow
    seq             c1, d.perpetual_timer_started, 0
    bal.c1          r7, tcp_tx_start_perpetual_timer
    phvwr           p.common_phv_debug_dol_bypass_barco, d.debug_dol_tx[TCP_TX_DDOL_BYPASS_BARCO_BIT]
    tblwr.f         d.{ci_1}.hx, d.{pi_1}.hx
send_ack_doorbell:

    /*
     * Ring doorbell to set CI
     */
    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA_NOP(k.p4_txdma_intr_qid)
    memwr.dx        r4, r3

    // This table need not be locked since it is read-only.
    // Moreover it should not be locked to prevent the bypass
    // cache from being used (bypass cache cannot be used,
    // since the contents are written from rx pipeline)
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS,
                        tcp_tx_read_rx2tx_shared_extra_stage1_start,
                        k.p4_txdma_intr_qstate_addr,
                        TCP_TCB_RX2TX_SHARED_EXTRA_OFFSET, TABLE_SIZE_512_BITS)
    phvwrpair       p.common_phv_fid, k.p4_txdma_intr_qid, \
                        p.common_phv_qstate_addr, k.p4_txdma_intr_qstate_addr

#ifndef HW
    smeqb           c2, d.debug_dol_tx, TCP_TX_DDOL_DONT_SEND_ACK, TCP_TX_DDOL_DONT_SEND_ACK
    phvwri.c2       p.common_phv_debug_dol_dont_send_ack, 1
#endif
    phvwrpair.e     p.common_phv_pending_rx2tx, 1, \
                        p.common_phv_pending_ack_send, 1

pending_send_ack_end:
    nop.e
    nop

/******************************************************************************
 * tcp_tx_clean_retx
 *****************************************************************************/
tcp_tx_clean_retx:
    bbeq            d.clean_retx_pending[0], 1, clean_retx_drop
    // if sesq_retx_ci has reached sesq_ci, there is nothing to
    // retransmit. Quit.
    seq             c1, d.sesq_retx_ci, d.{ci_0}.hx
    seq             c2, d.asesq_retx_ci, d.{ci_4}.hx
    bcf             [c1 & c2], clean_retx_flush_and_drop

    phvwrpair       p.common_phv_fid, k.p4_txdma_intr_qid, \
                        p.common_phv_qstate_addr, k.p4_txdma_intr_qstate_addr
    //tblwr           d.debug_dol_tblsetaddr, d.sesq_retx_ci[7:0]
    tblwr.f         d.clean_retx_pending, CLEAN_RETX_PENDING_MASK
    phvwr           p.to_s3_window_not_restricted, d.sesq_tx_ci[15]

    // This table need not be locked since it is read-only.
    // Moreover it should not be locked to prevent the bypass
    // cache from being used (bypass cache cannot be used,
    // since the contents are written from rx pipeline)
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS,
                        tcp_tx_read_rx2tx_shared_extra_clean_retx_stage1_start,
                        k.p4_txdma_intr_qstate_addr,
                        TCP_TCB_RX2TX_SHARED_EXTRA_OFFSET, TABLE_SIZE_512_BITS)
    phvwr           p.common_phv_debug_dol_bypass_barco, d.debug_dol_tx[TCP_TX_DDOL_BYPASS_BARCO_BIT]
    phvwrpair       p.common_phv_pending_retx_cleanup, PENDING_RETX_CLEANUP, \
                        p.common_phv_pending_rx2tx, 1
    phvwr           p.to_s3_clean_retx_pi, d.{pi_7}.hx
#ifndef HW
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_START_RETX_TIMER, TCP_TX_DDOL_START_RETX_TIMER
    phvwri.!c1      p.common_phv_debug_dol_dont_start_retx_timer, 1
#endif

pending_rx2tx_snd_una_update:
    seq             c1, d.asesq_retx_ci, d.{ci_4}.hx
    b.c1            pending_rx2tx_clean_sesq

pending_rx2tx_clean_asesq:
    /*
     * Launch asesq entry read with asesq RETX CI as index
     */
    // asesq_base = sesq_base - number of sesq slots
    sub             r3, d.{sesq_base}.wx, ASIC_SESQ_RING_SLOTS, NIC_SESQ_ENTRY_SIZE_SHIFT
    add             r3, r3, d.asesq_retx_ci, NIC_SESQ_ENTRY_SIZE_SHIFT

    // pkts to free = 1, don't batch asesq free
    phvwr           p.t0_s2s_clean_retx_num_retx_pkts, 1

    phvwri          p.common_phv_pending_asesq, 1
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, tcp_tx_sesq_read_retx_ci_stage1_start,
                     r3, TABLE_SIZE_512_BITS)
    b               pending_rx2tx_clean_sesq_done
    /*
     * Relay ASESQ CI to S3
     */
    phvwr           p.to_s3_sesq_retx_ci, d.asesq_retx_ci

pending_rx2tx_clean_sesq:
    /*
     * Relay SESQ CI to S3 for updation in the TLS-CB
     */
    phvwr           p.to_s3_sesq_retx_ci, d.sesq_retx_ci

    // if tx_ci is invalid
    //      pkts to free = min(ci-retx_ci, distance to end of sesq ring)
    // else
    //      pkts to free = min(tx_ci-retx_ci, distance to end of sesq ring)
    seq             c1, d.sesq_tx_ci, TCP_TX_INVALID_SESQ_TX_CI
    sub.c1          r1, d.{ci_0}.hx, d.sesq_retx_ci
    sub.!c1         r1, d.sesq_tx_ci, d.sesq_retx_ci
    sub             r2, ASIC_SESQ_RING_SLOTS, d.sesq_retx_ci
    slt             c1, r2, r1[ASIC_SESQ_RING_SLOTS_SHIFT-1:0]
    add.c1          r1, r0, r2
    // free 8 at the most
    slt             c1, 8, r1[ASIC_SESQ_RING_SLOTS_SHIFT-1:0]
    add.c1          r1, r0, 8
    phvwr           p.t0_s2s_clean_retx_num_retx_pkts, r1[ASIC_SESQ_RING_SLOTS_SHIFT-1:0]

    /*
     * Launch sesq entry read with RETX CI as index
     */
    add             r3, d.{sesq_base}.wx, d.sesq_retx_ci, NIC_SESQ_ENTRY_SIZE_SHIFT
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, tcp_tx_sesq_read_retx_ci_stage1_start,
                     r3, TABLE_SIZE_512_BITS)

pending_rx2tx_clean_sesq_done:
    // barrier is set to non-zero here in stage 0, and set to 0 in stage 3.
    // Sometimes we see that barrier memwr to 0 in stage 3 overtakes the tblwr
    // in stage 0. This wrfence is to preserve the order
    wrfence.e

    nop

clean_retx_flush_and_drop:
    tblwr.f         d.{ci_7}.hx, d.{pi_7}.hx
    /*
     * Ring doorbell to set CI
     */
    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA_NOP(k.p4_txdma_intr_qid)
    memwr.dx        r4, r3

clean_retx_drop:
    phvwri          p.app_header_table0_valid, 0;
    phvwri.e        p.p4_intr_global_drop, 1
    nop



/******************************************************************************
 * tcp_tx_fast_retrans
 *****************************************************************************/
tcp_tx_fast_retrans:
#ifndef HW
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_START_RETX_TIMER, TCP_TX_DDOL_START_RETX_TIMER
    phvwri.!c1      p.common_phv_debug_dol_dont_start_retx_timer, 1
#endif
    phvwr           p.common_phv_pending_fast_retx, 1
    tblwr.f         d.{ci_6}.hx, d.{pi_6}.hx

    // This table need not be locked since it is read-only.
    // Moreover it should not be locked to prevent the bypass
    // cache from being used (bypass cache cannot be used,
    // since the contents are written from rx pipeline)
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS,
                        tcp_tx_read_rx2tx_shared_extra_stage1_start,
                        k.p4_txdma_intr_qstate_addr,
                        TCP_TCB_RX2TX_SHARED_EXTRA_OFFSET, TABLE_SIZE_512_BITS)
    phvwrpair       p.common_phv_fid, k.p4_txdma_intr_qid, \
                        p.common_phv_qstate_addr, k.p4_txdma_intr_qstate_addr

    /*
     * Ring doorbell to set CI
     */

    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    // data will be in r3
    CAPRI_RING_DOORBELL_DATA_NOP(k.p4_txdma_intr_qid)
    memwr.dx        r4, r3

    /*
     * Launch sesq entry ready with RETX CI as index
     */
    add             r3, d.{sesq_base}.wx, d.sesq_retx_ci, NIC_SESQ_ENTRY_SIZE_SHIFT
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, tcp_tx_sesq_read_ci_stage1_start,
                     r3, TABLE_SIZE_64_BITS)

    nop.e
    nop


/******************************************************************************
 * tcp_tx_ft_expired
 *****************************************************************************/
tcp_tx_ft_expired:
    seq             c1, d.rto_deadline, 0
    seq             c2, d.ato_deadline, 0
    seq             c3, d.idle_deadline, 0
    bcf             [c1 & c2 & c3], tcp_tx_stop_perpetual_timer_and_exit
    nop
#ifdef HW
    bal             r7, tcp_tx_start_perpetual_timer
#else
    // Don't restart perpetual timer in simulation unless testing retx timer
    // and rto is non-zero
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_START_RETX_TIMER, TCP_TX_DDOL_START_RETX_TIMER
    sne.c1          c1, d.rto_deadline, 0
    bal.c1          r7, tcp_tx_start_perpetual_timer
#endif
    /*
     * c4 ==> both ato and rto are going to expire
     * Decrement both ato and rto, if not both are going to expire (!c4)
     *
     * decrement rto, and check for retransmission timer expired (c1)
     *
     * decrement rto, and check for retransmission timer expired (c2)
     */

    /*
     * Check if rto-- == 1
     * If TRUE, we want to do timeout handling
     */
    seq             c1, d.rto_deadline, 1
    tblssub         d.rto_deadline, 1

    /*
     * Check if ato-- == 1
     * If TRUE, we want to do delack handling
     */
    seq             c2, d.ato_deadline, 1
    setcf           c4, [c1 & c2]
    tblssub.!c4     d.ato_deadline, 1
    seq             c3, d.idle_deadline, 1
    tblssub         d.idle_deadline, 1
    tblwr.f         d.{ci_2}.hx, d.{pi_2}.hx

    // Ring doorbell to clear scheduler if necessary
    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    // data will be in r3
    CAPRI_RING_DOORBELL_DATA_NOP(k.p4_txdma_intr_qid)
    bcf             [!c1 & !c2 & !c3], tcp_tx_rx2tx_end
    memwr.dx        r4, r3

    b.c1            tcp_tx_retx_timer_expired
    nop
    b.c2            tcp_tx_del_ack_timer_expired
    nop

    // c3 = true (idle timeout)

tcp_tx_idle_timer_expired:
    phvwr           p.common_phv_pending_idle, 1

    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS,
                        tcp_tx_read_rx2tx_shared_extra_idle_start,
                        k.p4_txdma_intr_qstate_addr,
                        TCP_TCB_RX2TX_SHARED_EXTRA_OFFSET, TABLE_SIZE_512_BITS)
    phvwrpair.e     p.common_phv_fid, k.p4_txdma_intr_qid, \
                        p.common_phv_qstate_addr, k.p4_txdma_intr_qstate_addr
    nop

tcp_tx_del_ack_timer_expired:
    phvwri          p.common_phv_pending_rx2tx, 1
    phvwr           p.common_phv_pending_ack_send, 1

    // This table need not be locked since it is read-only.
    // Moreover it should not be locked to prevent the bypass
    // cache from being used (bypass cache cannot be used,
    // since the contents are written from rx pipeline)
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS,
                        tcp_tx_read_rx2tx_shared_extra_stage1_start,
                        k.p4_txdma_intr_qstate_addr,
                        TCP_TCB_RX2TX_SHARED_EXTRA_OFFSET, TABLE_SIZE_512_BITS)
    phvwrpair.e     p.common_phv_fid, k.p4_txdma_intr_qid, \
                        p.common_phv_qstate_addr, k.p4_txdma_intr_qstate_addr
    nop

tcp_tx_stop_perpetual_timer_and_exit:
    tblwr           d.{ci_2}.hx, d.{pi_2}.hx
    tblwr.f         d.perpetual_timer_started, 0
    // Ring doorbell to clear scheduler if necessary
    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    // data will be in r3
    CAPRI_RING_DOORBELL_DATA_NOP(k.p4_txdma_intr_qid)
    memwr.dx.e      r4, r3
    phvwri          p.p4_intr_global_drop, 1

tcp_tx_retx_timer_expired:
#ifndef HW
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_START_RETX_TIMER, TCP_TX_DDOL_START_RETX_TIMER
    phvwr.!c1       p.common_phv_debug_dol_dont_start_retx_timer, 1
#endif

    phvwr           p.common_phv_pending_rto, 1
    seq             c1, d.sesq_retx_ci, d.{ci_0}.hx
    seq             c2, d.asesq_retx_ci, d.{ci_4}.hx
    bcf             [c1 & c2], tcp_tx_rx2tx_end

    // This table need not be locked since it is read-only.
    // Moreover it should not be locked to prevent the bypass
    // cache from being used (bypass cache cannot be used,
    // since the contents are written from rx pipeline)
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS,
                        tcp_tx_read_rx2tx_shared_extra_stage1_start,
                        k.p4_txdma_intr_qstate_addr,
                        TCP_TCB_RX2TX_SHARED_EXTRA_OFFSET, TABLE_SIZE_512_BITS)
    phvwrpair       p.common_phv_fid, k.p4_txdma_intr_qid, \
                        p.common_phv_qstate_addr, k.p4_txdma_intr_qstate_addr

    seq             c1, d.asesq_retx_ci, d.{ci_4}.hx
    b.c1            tcp_tx_retx_timer_expired_launch_sesq
tcp_tx_retx_timer_expired_launch_asesq:
    /*
     * Launch asesq entry ready with RETX CI as index
     */
    // asesq_base = sesq_base - number of sesq slots
    sub             r3, d.{sesq_base}.wx, ASIC_SESQ_RING_SLOTS, NIC_SESQ_ENTRY_SIZE_SHIFT
    add             r3, r3, d.asesq_retx_ci, NIC_SESQ_ENTRY_SIZE_SHIFT
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, tcp_tx_sesq_read_ci_stage1_start,
                     r3, TABLE_SIZE_64_BITS)
    b               tcp_tx_retx_timer_expired_done
    phvwri          p.common_phv_pending_asesq, 1
tcp_tx_retx_timer_expired_launch_sesq:
    /*
     * Launch sesq entry ready with RETX CI as index
     */
    add             r3, d.{sesq_base}.wx, d.sesq_retx_ci, NIC_SESQ_ENTRY_SIZE_SHIFT
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, tcp_tx_sesq_read_ci_stage1_start,
                     r3, TABLE_SIZE_64_BITS)

tcp_tx_retx_timer_expired_done:
#ifndef HW
    // DEBUG code
    // If we are testing timer full case, don't start the timer again to
    // prevent an endless loop. We do need to process one timer to test
    // the timer full case (in the timer full case, the timer expires
    // immediately)
    phvwr           p.common_phv_debug_dol_dont_start_retx_timer, d.debug_dol_tx[TCP_TX_DDOL_FORCE_TIMER_FULL_BIT]
#endif

    nop.e
    nop

/******************************************************************************
 * tcp_tx_del_ack
 *****************************************************************************/
tcp_tx_del_ack:
    // start perpetual timer if necessary for the flow
    seq             c1, d.perpetual_timer_started, 0
    bal.c1          r7, tcp_tx_start_perpetual_timer
    nop
    tblwr.f         d.{ci_3}.hx, d.{pi_3}.hx
del_ack_doorbell:

    /*
     * Ring doorbell to clear scheduler
     */
    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA_NOP(k.p4_txdma_intr_qid)
    memwr.dx.e      r4, r3
    phvwri          p.p4_intr_global_drop, 1


/******************************************************************************
 * tcp_tx_abort
 *****************************************************************************/
tcp_tx_abort:
tcp_tx_rx2tx_end:
    phvwri.e        p.p4_intr_global_drop, 1
    CAPRI_CLEAR_TABLE_VALID(0)

/******************************************************************************
 * tcp_tx_start_perpetual_timer
 *****************************************************************************/
tcp_tx_start_perpetual_timer:
#ifndef HW
    // in simulation don't start timers unless testing them
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_START_DEL_ACK_TIMER, TCP_TX_DDOL_START_DEL_ACK_TIMER
    smeqb.!c1       c1, d.debug_dol_tx, TCP_TX_DDOL_START_RETX_TIMER, TCP_TX_DDOL_START_RETX_TIMER
    jr.!c1          r7
#endif
    tblwr           d.perpetual_timer_started, 1
    addi            r4, r0, CAPRI_FAST_TIMER_ADDR(LIF_TCP)
    // result will be in r3
    CAPRI_TIMER_DATA(0, k.p4_txdma_intr_qid, TCP_SCHED_RING_FAST_TIMER, TCP_TIMER_TICK)
    jr              r7
    memwr.dx        r4, r3

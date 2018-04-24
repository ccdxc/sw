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
    .param          tcp_tx_sesq_read_ci_stage1_start
    .param          tcp_tx_process_pending_start
    .param          tcp_tx_process_read_xmit_start

tcp_tx_read_rx2tx_shared_process:
    // This table need not be locked since it is read-only.
    // Moreover it should not be locked to prevent the bypass
    // cache from being used (bypass cache cannot be used,
    // since the contents are written from rx pipeline)
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS,
                        tcp_tx_read_rx2tx_shared_extra_stage1_start,
                        k.p4_txdma_intr_qstate_addr,
                        TCP_TCB_RX2TX_SHARED_EXTRA_OFFSET, TABLE_SIZE_512_BITS)

    add         r7, r7, r0 // debug only

	.brbegin
        // priorities are 0 (highest) to 7 (lowest)
        // The rightmost value specifies the priority of r7[0]
        // tx ring (5) is highest priority so that we finish pending activity
	    brpri		r7[5:0], [0,3,5,4,1,2]
	    nop
	        .brcase 0
	            b tcp_tx_launch_sesq
	            nop
	        .brcase 1
	            b tcp_tx_launch_pending_rx2tx
	            nop
	        .brcase 2
	            b tcp_tx_ft_expired
	            nop
	        .brcase 3
	            b tcp_tx_st_expired
	            nop
	        .brcase 4
	            b tcp_tx_launch_asesq
	            nop
	        .brcase 5
	            b tcp_tx_launch_pending_tx
	            nop
            .brcase 6
	            b tcp_tx_rx2tx_abort
	            nop
	.brend

/******************************************************************************
 * tcp_tx_launch_sesq
 *****************************************************************************/
tcp_tx_launch_sesq:
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_DONT_TX, TCP_TX_DDOL_DONT_TX
    phvwri.c1       p.common_phv_debug_dol_dont_tx, 1
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_DONT_START_RETX_TIMER, TCP_TX_DDOL_DONT_START_RETX_TIMER
    phvwri.c1       p.common_phv_debug_dol_dont_start_retx_timer, 1
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_FORCE_TBL_SETADDR, TCP_TX_DDOL_FORCE_TBL_SETADDR
    phvwri.c1       p.common_phv_debug_dol_force_tbl_setaddr, 1
    add             r3, d.{sesq_base}.wx, d.{ci_0}.hx, NIC_SESQ_ENTRY_SIZE_SHIFT
    phvwr           p.to_s1_sesq_ci_addr, r3

    /* Check if we have pending del ack timer (fast timer)
     * and cancel if running. The cancel is done by setting ci = pi
     */
    sne             c1, d.{pi_2}.hx, d.ft_pi
    tblwr.c1        d.{ci_2}, d.{ft_pi}.hx

    tblmincri.f     d.{ci_0}.hx, CAPRI_SESQ_RING_SLOTS_SHIFT, 1

    phvwri          p.common_phv_pending_sesq, 1
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, tcp_tx_sesq_read_ci_stage1_start,
                     r3, TABLE_SIZE_64_BITS)

    /*
     * Ring doorbell to set CI if pi == ci
     */
    seq             c1, d.{ci_0}.hx, d.{pi_0}.hx
    b.!c1           tcp_tx_launch_sesq_end

    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, TCP_SCHED_RING_SESQ, 0)
    memwr.dx        r4, r3

tcp_tx_launch_sesq_end:
    // DEBUG ONLY code
    add             r1, k.p4_txdma_intr_qstate_addr, TCP_DDOL_TBLADDR_SHIFT_OFFSET
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_FORCE_TBL_SETADDR, TCP_TX_DDOL_FORCE_TBL_SETADDR
    tblsetaddr.c1   r1, 2
    tblwr           d.debug_dol_tblsetaddr, TCP_DDOL_TBLADDR_VALUE
    // END of DEBUG code
    nop.e
    nop


/******************************************************************************
 * tcp_tx_launch_asesq
 *****************************************************************************/
tcp_tx_launch_asesq:
    phvwri          p.common_phv_pending_asesq, 1
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_DONT_TX, TCP_TX_DDOL_DONT_TX
    phvwri.c1       p.common_phv_debug_dol_dont_tx, 1
    and             r1, d.{ci_4}.hx, (CAPRI_ASESQ_RING_SLOTS - 1)
    add             r3, d.{asesq_base}.wx, r1, NIC_SESQ_ENTRY_SIZE_SHIFT
    tbladd.f        d.{ci_4}.hx, 1
    phvwr           p.to_s1_sesq_ci_addr, r3
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, tcp_tx_sesq_read_ci_stage1_start,
                     r3, TABLE_SIZE_64_BITS)

    /*
     * Ring doorbell to set CI if pi == ci
     */
    seq             c1, d.{ci_4}.hx, d.{pi_4}.hx
    b.!c1           tcp_tx_launch_asesq_end

    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, TCP_SCHED_RING_ASESQ, 0)
    memwr.dx        r4, r3

tcp_tx_launch_asesq_end:
    nop.e
    nop


/******************************************************************************
 * tcp_tx_launch_pending_tx
 *****************************************************************************/
tcp_tx_launch_pending_tx:
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_BYPASS_BARCO, TCP_TX_DDOL_BYPASS_BARCO
    phvwri.c1       p.common_phv_debug_dol_bypass_barco, 1

    /*
     * saved_pending_ack_send = 0 means we got scheduled to
     * clean retx
     */
    seq             c1, d.saved_pending_ack_send, 1
    b.!c1           pending_tx_snd_una_update
    tbladd.f        d.{ci_5}.hx, 1
    tblwr           d.saved_pending_ack_send, 0
    phvwri          p.common_phv_pending_rx2tx, 1
    phvwr           p.common_phv_pending_ack_send, 1
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_DONT_SEND_ACK, \
                        TCP_TX_DDOL_DONT_SEND_ACK
    phvwri.c1       p.common_phv_debug_dol_dont_send_ack, 1
    b               pending_tx_ring_doorbell
    nop

pending_tx_snd_una_update:
    /*
     * pending stage is only to read retx_next_desc currently, which is
     * only used in retx cleanup, so don't launch the stage if this
     * is not a snd_una_update
     */

    phvwr           p.common_phv_rx_flag, FLAG_SND_UNA_ADVANCED

    CAPRI_NEXT_TABLE_READ_OFFSET(1, TABLE_LOCK_DIS,
                        tcp_tx_process_pending_start,
                        k.p4_txdma_intr_qstate_addr,
                        TCP_TCB_RETX_OFFSET, TABLE_SIZE_512_BITS)

    CAPRI_NEXT_TABLE_READ_OFFSET(2, TABLE_LOCK_DIS,
                        tcp_tx_process_read_xmit_start,
                        k.p4_txdma_intr_qstate_addr,
                        TCP_TCB_XMIT_OFFSET, TABLE_SIZE_512_BITS)

pending_tx_ring_doorbell:
    /*
     * Ring doorbell to set CI if pi == ci
     */
    seq             c1, d.{ci_5}.hx, d.{pi_5}.hx
    b.!c1           pending_tx_end

    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP,
                        DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, TCP_SCHED_RING_PENDING_TX,
                        0)
    memwr.dx        r4, r3

pending_tx_end:
    nop.e
    nop


/******************************************************************************
 * tcp_tx_launch_pending_rx2tx
 *****************************************************************************/
tcp_tx_launch_pending_rx2tx:
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_BYPASS_BARCO, TCP_TX_DDOL_BYPASS_BARCO
    phvwri.c1       p.common_phv_debug_dol_bypass_barco, 1

    seq             c1, d.pending_ack_send, 1
    smeqb           c2, d.rx_flag, FLAG_SND_UNA_ADVANCED, FLAG_SND_UNA_ADVANCED
    setcf           c3, [c1 & c2]
    b.!c3           pending_one_rx2tx
    phvwri          p.common_phv_pending_rx2tx, 1

    /*
     * Two pending activities, save one for later and reschedule
     */
    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_INC,
                        DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, TCP_SCHED_RING_PENDING_TX, 0)
    memwr.dx        r4, r3
    tblwr.f         d.saved_pending_ack_send, 1

pending_one_rx2tx:
    phvwr.c2        p.common_phv_rx_flag, d.rx_flag
    tblwr.c2        d.rx_flag, 0
    b.c2            pending_rx2tx_snd_una_update
    tbladd.f        d.{ci_1}.hx, 1
    b.c1            pending_ack_send

pending_rx2tx_snd_una_update:
    /*
     * pending stage is only to read retx_next_desc currently, which is
     * only used in retx cleanup, so don't launch the stage if this
     * is not a snd_una_update
     */

    CAPRI_NEXT_TABLE_READ_OFFSET(1, TABLE_LOCK_DIS,
                        tcp_tx_process_pending_start,
                        k.p4_txdma_intr_qstate_addr,
                        TCP_TCB_RETX_OFFSET, TABLE_SIZE_512_BITS)

    CAPRI_NEXT_TABLE_READ_OFFSET(2, TABLE_LOCK_DIS,
                        tcp_tx_process_read_xmit_start,
                        k.p4_txdma_intr_qstate_addr,
                        TCP_TCB_XMIT_OFFSET, TABLE_SIZE_512_BITS)

    b               pending_rx2tx_doorbell

pending_ack_send:
    phvwr           p.common_phv_pending_ack_send, d.pending_ack_send
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_DONT_SEND_ACK, TCP_TX_DDOL_DONT_SEND_ACK
    phvwri.c1       p.common_phv_debug_dol_dont_send_ack, 1

pending_rx2tx_doorbell:

    /*
     * Ring doorbell to set CI if pi == ci
     */
    seq             c1, d.{ci_1}.hx, d.{pi_1}.hx
    b.!c1           pending_rx2tx_end

    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, TCP_SCHED_RING_PENDING_RX2TX, 0)
    memwr.dx        r4, r3

pending_rx2tx_end:
    nop.e
    nop


/******************************************************************************
 * tcp_tx_st_expired
 *****************************************************************************/
tcp_tx_st_expired:
    phvwr           p.common_phv_pending_rto, 1
    phvwr           p.t0_s2s_rto_pi, d.{pi_3}.hx
    tbladd.f        d.{ci_3}.hx, 1

    /*
     * Ring doorbell to set CI if pi == ci
     */
    seq             c1, d.{ci_3}.hx, d.{pi_3}.hx
    b.!c1           tcp_tx_st_expired_end

    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    // data will be in r3
    CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, TCP_SCHED_RING_ST, 0)
    memwr.dx        r4, r3

tcp_tx_st_expired_end:
    // DEBUG code
    // If we are testing timer full case, don't start the timer again to
    // prevent an endless loop. We do need to process one timer to test
    // the timer full case (in the timer full case, the timer expires
    // immediately)
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_FORCE_TIMER_FULL, TCP_TX_DDOL_FORCE_TIMER_FULL
    phvwri.c1       p.common_phv_debug_dol_dont_start_retx_timer, 1

    nop.e
    nop


/******************************************************************************
 * tcp_tx_ft_expired
 *****************************************************************************/
tcp_tx_ft_expired:
    phvwri          p.common_phv_pending_rx2tx, 1
    phvwr           p.common_phv_pending_ack_send, d.pending_ack_send
    /* Check if SW PI (ft_pi) == timer pi
     * If TRUE, we want to process this timer, else its an old timer expiry and we can
     * ignore it
     */
    seq             c1, d.{pi_2}.hx, d.ft_pi

    /* For old timer expiry, no more processing, invalidate table bits */
    phvwri.!c1      p.app_header_table0_valid, 0

tcp_tx_cancel_fast_timer:
    /* Store new CI in r5; new ci = sw pi */
    add             r5, r0, d.ft_pi
    /* For old timer expiry, set new ci == sw pi - 1 */
    sub.!c1         r5, r5, 1

    /* Ring FT doorbell to update CI
     * store address in r4
     */
    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_CIDX_SET, DB_SCHED_UPD_EVAL, 0, LIF_TCP)

    // data will be in r3
    CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, TCP_SCHED_RING_FT, r5)
    memwr.dx.e      r4, r3
    tbladd          d.{ci_2}.hx, 1
    nop
    

/******************************************************************************
 * tcp_tx_rx2tx_abort
 *****************************************************************************/
tcp_tx_rx2tx_abort:
    phvwri          p.p4_intr_global_drop, 1
    nop.e
    CAPRI_CLEAR_TABLE_VALID(0)

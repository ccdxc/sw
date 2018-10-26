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
    .param          tcp_tx_process_read_xmit_start

tcp_tx_read_rx2tx_shared_process:
	.brbegin
        // priorities are 0 (highest) to 7 (lowest)
        // The rightmost value specifies the priority of r7[0]
        // tx ring (5) is highest priority so that we finish pending activity
	    brpri		r7[7:0], [1,3,0,5,7,6,2,4]
	    nop
	        .brcase 0
	            b tcp_tx_launch_sesq            // prio 4
	            nop
	        .brcase 1
	            b tcp_tx_send_ack               // prio 1
	            nop
	        .brcase 2
	            b tcp_tx_ft_expired             // prio 6
	            nop
	        .brcase 3
	            b tcp_tx_retx_timer_expired     // prio 7
	            nop
	        .brcase 4
	            b tcp_tx_launch_asesq           // prio 5
	            nop
	        .brcase 5
	            b tcp_tx_launch_pending_tx      // prio 0
	            nop
	        .brcase 6
	            b tcp_tx_fast_retrans           // prio 3
	            nop
            .brcase 7
	            b tcp_tx_clean_retx             // prio 2
	            nop
            .brcase 8
	            b tcp_tx_rx2tx_abort
	            nop
	.brend

/******************************************************************************
 * tcp_tx_launch_sesq
 *****************************************************************************/
tcp_tx_launch_sesq:
#ifndef HW
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_DONT_TX, TCP_TX_DDOL_DONT_TX
    phvwri.c1       p.common_phv_debug_dol_dont_tx, 1
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_DONT_START_RETX_TIMER, TCP_TX_DDOL_DONT_START_RETX_TIMER
    phvwri.c1       p.common_phv_debug_dol_dont_start_retx_timer, 1
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_FORCE_TBL_SETADDR, TCP_TX_DDOL_FORCE_TBL_SETADDR
    phvwri.c1       p.common_phv_debug_dol_force_tbl_setaddr, 1
#endif
    add             r5, d.{sesq_base}.wx, d.{ci_0}.hx, NIC_SESQ_ENTRY_SIZE_SHIFT

    /* Check if we have pending del ack timer (fast timer)
     * and cancel if running. The cancel is done by setting ci = pi
     */
    sne             c1, d.{pi_2}.hx, d.ft_pi
    tblwr.c1        d.{ci_2}, d.{ft_pi}.hx
    tblwr           d.old_ack_no, d.rcv_nxt

    tblmincri.f     d.{ci_0}.hx, CAPRI_SESQ_RING_SLOTS_SHIFT, 1
    phvwr           p.common_phv_debug_dol_bypass_barco, d.debug_dol_tx[TCP_TX_DDOL_BYPASS_BARCO_BIT]

    /*
     * Ring doorbell to set CI if pi == ci
     */
    seq             c1, d.{ci_0}.hx, d.{pi_0}.hx
    b.!c1           tcp_tx_skip_sesq_doorbell

    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    CAPRI_RING_DOORBELL_DATA_NOP(k.p4_txdma_intr_qid, TCP_SCHED_RING_SESQ)
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
    phvwr           p.to_s6_rcv_nxt, d.rcv_nxt
    phvwrpair       p.common_phv_fid, k.p4_txdma_intr_qid, \
                        p.common_phv_qstate_addr, k.p4_txdma_intr_qstate_addr
    phvwr           p.t0_s2s_state, d.state

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


/******************************************************************************
 * tcp_tx_launch_asesq
 *****************************************************************************/
tcp_tx_launch_asesq:
    phvwri          p.common_phv_pending_asesq, 1
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_DONT_TX, TCP_TX_DDOL_DONT_TX
    phvwri.c1       p.common_phv_debug_dol_dont_tx, 1
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_DONT_START_RETX_TIMER, TCP_TX_DDOL_DONT_START_RETX_TIMER
    phvwri.c1       p.common_phv_debug_dol_dont_start_retx_timer, 1

    // asesq_base = sesq_base - number of sesq slots
    sub             r3, d.{sesq_base}.wx, CAPRI_SESQ_RING_SLOTS, NIC_SESQ_ENTRY_SIZE_SHIFT

    and             r1, d.{ci_4}.hx, (CAPRI_ASESQ_RING_SLOTS - 1)
    add             r3, r3, r1, NIC_SESQ_ENTRY_SIZE_SHIFT
    tbladd.f        d.{ci_4}.hx, 1

    // This table need not be locked since it is read-only.
    // Moreover it should not be locked to prevent the bypass
    // cache from being used (bypass cache cannot be used,
    // since the contents are written from rx pipeline)
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS,
                        tcp_tx_read_rx2tx_shared_extra_stage1_start,
                        k.p4_txdma_intr_qstate_addr,
                        TCP_TCB_RX2TX_SHARED_EXTRA_OFFSET, TABLE_SIZE_512_BITS)
    phvwr           p.to_s6_rcv_nxt, d.rcv_nxt
    phvwrpair       p.common_phv_fid, k.p4_txdma_intr_qid, \
                        p.common_phv_qstate_addr, k.p4_txdma_intr_qstate_addr
    phvwr           p.t0_s2s_state, d.state
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, tcp_tx_sesq_read_ci_stage1_start,
                     r3, TABLE_SIZE_64_BITS)

    /*
     * Ring doorbell to set CI if pi == ci
     */
    seq             c1, d.{ci_4}.hx, d.{pi_4}.hx
    b.!c1           tcp_tx_launch_asesq_end

    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA_NOP(k.p4_txdma_intr_qid, TCP_SCHED_RING_ASESQ)
    memwr.dx        r4, r3

tcp_tx_launch_asesq_end:
    nop.e
    nop


/******************************************************************************
 * tcp_tx_launch_pending_tx
 *****************************************************************************/
tcp_tx_launch_pending_tx:
    tblwr           d.{ci_5}.hx, d.{pi_5}.hx
#ifndef HW
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_DONT_START_RETX_TIMER, TCP_TX_DDOL_DONT_START_RETX_TIMER
    phvwri.c1       p.common_phv_debug_dol_dont_start_retx_timer, 1

    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_DONT_SEND_ACK, \
                        TCP_TX_DDOL_DONT_SEND_ACK
    phvwri.c1       p.common_phv_debug_dol_dont_send_ack, 1
#endif

pending_tx_snd_una_update:

    seq             c1, d.asesq_retx_ci, d.{ci_4}.hx
    b.!c1           pending_tx_clean_asesq

pending_tx_clean_sesq:
    // if sesq_retx_ci has reached sesq_ci, there is nothing to
    // retransmit. Quit.
    seq             c1, d.sesq_retx_ci, d.{ci_0}.hx
    b.c1            pending_tx_ring_doorbell_and_drop
    /*
     * Launch sesq entry read with RETX CI as index
     */
    add             r5, d.{sesq_base}.wx, d.sesq_retx_ci, NIC_SESQ_ENTRY_SIZE_SHIFT
    tblmincri.f     d.sesq_retx_ci, CAPRI_SESQ_RING_SLOTS_SHIFT, 1

pending_tx_sesq_ring_doorbell:
    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP,
                        DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA_NOP(k.p4_txdma_intr_qid, TCP_SCHED_RING_PENDING_TX)
    memwr.dx        r4, r3

    phvwr           p.common_phv_debug_dol_bypass_barco, d.debug_dol_tx[TCP_TX_DDOL_BYPASS_BARCO_BIT]
    /*
     * Relay SESQ CI to S3 for updation in the TLS-CB
     */
    phvwr           p.to_s3_sesq_retx_ci, d.sesq_retx_ci

pending_tx_sesq_launch_next:
    // This table need not be locked since it is read-only.
    // Moreover it should not be locked to prevent the bypass
    // cache from being used (bypass cache cannot be used,
    // since the contents are written from rx pipeline)
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS,
                        tcp_tx_read_rx2tx_shared_extra_stage1_start,
                        k.p4_txdma_intr_qstate_addr,
                        TCP_TCB_RX2TX_SHARED_EXTRA_OFFSET, TABLE_SIZE_512_BITS)
    phvwr           p.to_s6_rcv_nxt, d.rcv_nxt
    phvwrpair       p.common_phv_fid, k.p4_txdma_intr_qid, \
                        p.common_phv_qstate_addr, k.p4_txdma_intr_qstate_addr
    phvwr           p.t0_s2s_state, d.state
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, tcp_tx_sesq_read_ci_stage1_start,
                     r5, TABLE_SIZE_64_BITS)

pending_tx_clean_sesq_done:
    phvwrpair       p.common_phv_pending_retx_cleanup, PENDING_RETX_CLEANUP_TRIGGERED_FROM_TX, \
                        p.common_phv_pending_rx2tx, 1

    CAPRI_NEXT_TABLE_READ_OFFSET_e(2, TABLE_LOCK_DIS,
                        tcp_tx_process_read_xmit_start,
                        k.p4_txdma_intr_qstate_addr,
                        TCP_TCB_XMIT_OFFSET, TABLE_SIZE_512_BITS)
    nop

pending_tx_ring_doorbell_and_drop:
    phvwri          p.app_header_table0_valid, 0;
    phvwri          p.p4_intr_global_drop, 1
    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP,
                        DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA_NOP(k.p4_txdma_intr_qid, TCP_SCHED_RING_PENDING_TX)
    memwr.dx.e      r4, r3
    nop


pending_tx_clean_asesq:
    /*
     * Launch asesq entry read with asesq RETX CI as index
     */

    // asesq_base = sesq_base - number of sesq slots
    sub             r5, d.{sesq_base}.wx, CAPRI_SESQ_RING_SLOTS, NIC_SESQ_ENTRY_SIZE_SHIFT
    add             r5, r5, d.asesq_retx_ci, NIC_SESQ_ENTRY_SIZE_SHIFT
    tblmincri.f     d.asesq_retx_ci, CAPRI_ASESQ_RING_SLOTS_SHIFT, 1

pending_tx_asesq_ring_doorbell:
    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP,
                        DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA_NOP(k.p4_txdma_intr_qid, TCP_SCHED_RING_PENDING_TX)
    memwr.dx        r4, r3

    phvwr           p.common_phv_debug_dol_bypass_barco, d.debug_dol_tx[TCP_TX_DDOL_BYPASS_BARCO_BIT]
    /*
     * Relay SESQ CI to S3 for updation in the TLS-CB
     */
    phvwr           p.to_s3_sesq_retx_ci, d.sesq_retx_ci

pending_tx_asesq_launch_next:
    phvwri          p.common_phv_pending_asesq, 1
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, tcp_tx_sesq_read_ci_stage1_start,
                     r5, TABLE_SIZE_64_BITS)
    // This table need not be locked since it is read-only.
    // Moreover it should not be locked to prevent the bypass
    // cache from being used (bypass cache cannot be used,
    // since the contents are written from rx pipeline)
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS,
                        tcp_tx_read_rx2tx_shared_extra_stage1_start,
                        k.p4_txdma_intr_qstate_addr,
                        TCP_TCB_RX2TX_SHARED_EXTRA_OFFSET, TABLE_SIZE_512_BITS)
    phvwr           p.to_s6_rcv_nxt, d.rcv_nxt
    phvwrpair       p.common_phv_fid, k.p4_txdma_intr_qid, \
                        p.common_phv_qstate_addr, k.p4_txdma_intr_qstate_addr
    phvwr           p.t0_s2s_state, d.state
pending_tx_clean_asesq_done:
    phvwrpair       p.common_phv_pending_retx_cleanup, PENDING_RETX_CLEANUP_TRIGGERED_FROM_TX, \
                        p.common_phv_pending_rx2tx, 1

    CAPRI_NEXT_TABLE_READ_OFFSET(2, TABLE_LOCK_DIS,
                        tcp_tx_process_read_xmit_start,
                        k.p4_txdma_intr_qstate_addr,
                        TCP_TCB_XMIT_OFFSET, TABLE_SIZE_512_BITS)

pending_tx_asesq_end:
    nop.e
    nop


/******************************************************************************
 * tcp_tx_send_ack
 *****************************************************************************/
tcp_tx_send_ack:
    phvwr           p.common_phv_debug_dol_bypass_barco, d.debug_dol_tx[TCP_TX_DDOL_BYPASS_BARCO_BIT]

    /*
     * c1 = send ack
     *
     * if dup_ack_send, c1 = 1
     * else if old_ack_no != rcv_nxt, c1 = d.pending_ack_send
     */
    seq             c1, d.pending_dup_ack_send, 1
    sne.!c1         c1, d.old_ack_no, d.rcv_nxt          
    tblwr.c1        d.old_ack_no, d.rcv_nxt
    tblwr.f         d.{ci_1}.hx, d.{pi_1}.hx
send_ack_doorbell:

    /*
     * Ring doorbell to set CI
     */
    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA_NOP(k.p4_txdma_intr_qid, TCP_SCHED_RING_SEND_ACK)
    b.!c1           tcp_tx_rx2tx_end
    memwr.dx        r4, r3

    // This table need not be locked since it is read-only.
    // Moreover it should not be locked to prevent the bypass
    // cache from being used (bypass cache cannot be used,
    // since the contents are written from rx pipeline)
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS,
                        tcp_tx_read_rx2tx_shared_extra_stage1_start,
                        k.p4_txdma_intr_qstate_addr,
                        TCP_TCB_RX2TX_SHARED_EXTRA_OFFSET, TABLE_SIZE_512_BITS)
    phvwr           p.to_s6_rcv_nxt, d.rcv_nxt
    phvwrpair       p.common_phv_fid, k.p4_txdma_intr_qid, \
                        p.common_phv_qstate_addr, k.p4_txdma_intr_qstate_addr
    phvwr           p.t0_s2s_state, d.state

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
    tblwr.f         d.{ci_7}.hx, d.{pi_7}.hx
clean_retx_doorbell:

    /*
     * Ring doorbell to set CI
     */
    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA_NOP(k.p4_txdma_intr_qid, TCP_SCHED_RING_CLEAN_RETX)
    memwr.dx        r4, r3

    // This table need not be locked since it is read-only.
    // Moreover it should not be locked to prevent the bypass
    // cache from being used (bypass cache cannot be used,
    // since the contents are written from rx pipeline)
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS,
                        tcp_tx_read_rx2tx_shared_extra_stage1_start,
                        k.p4_txdma_intr_qstate_addr,
                        TCP_TCB_RX2TX_SHARED_EXTRA_OFFSET, TABLE_SIZE_512_BITS)
    phvwr           p.to_s6_rcv_nxt, d.rcv_nxt
    phvwrpair       p.common_phv_fid, k.p4_txdma_intr_qid, \
                        p.common_phv_qstate_addr, k.p4_txdma_intr_qstate_addr
    phvwr           p.t0_s2s_state, d.state
    phvwr           p.common_phv_debug_dol_bypass_barco, d.debug_dol_tx[TCP_TX_DDOL_BYPASS_BARCO_BIT]
    phvwrpair       p.common_phv_pending_retx_cleanup, PENDING_RETX_CLEANUP_TRIGGERED_FROM_RX, \
                        p.common_phv_pending_rx2tx, 1
#ifndef HW
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_DONT_START_RETX_TIMER, TCP_TX_DDOL_DONT_START_RETX_TIMER
    phvwri.c1       p.common_phv_debug_dol_dont_start_retx_timer, 1
#endif

pending_rx2tx_snd_una_update:
    /*
     * Relay SESQ CI to S3 for updation in the TLS-CB
     */
    phvwr           p.to_s3_sesq_retx_ci, d.sesq_retx_ci

    seq             c1, d.asesq_retx_ci, d.{ci_4}.hx
    b.c1            pending_rx2tx_clean_sesq

pending_rx2tx_clean_asesq:
    /*
     * Launch asesq entry read with asesq RETX CI as index
     */
    // asesq_base = sesq_base - number of sesq slots
    sub             r3, d.{sesq_base}.wx, CAPRI_SESQ_RING_SLOTS, NIC_SESQ_ENTRY_SIZE_SHIFT
    add             r3, r3, d.asesq_retx_ci, NIC_SESQ_ENTRY_SIZE_SHIFT
    //tblmincri.f     d.asesq_retx_ci, CAPRI_ASESQ_RING_SLOTS_SHIFT, 1
    phvwri          p.common_phv_pending_asesq, 1
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, tcp_tx_sesq_read_ci_stage1_start,
                     r3, TABLE_SIZE_64_BITS)
    b               pending_rx2tx_clean_sesq_done

pending_rx2tx_clean_sesq:
    // if sesq_retx_ci has reached sesq_ci, there is nothing to
    // retransmit. Quit.
    seq             c1, d.sesq_retx_ci, d.{ci_0}.hx
    b.c1            clean_retx_drop
    /*
     * Launch sesq entry read with RETX CI as index
     */
    add             r3, d.{sesq_base}.wx, d.sesq_retx_ci, NIC_SESQ_ENTRY_SIZE_SHIFT
    //tblmincri.f     d.sesq_retx_ci, CAPRI_SESQ_RING_SLOTS_SHIFT, 1
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, tcp_tx_sesq_read_ci_stage1_start,
                     r3, TABLE_SIZE_64_BITS)

pending_rx2tx_clean_sesq_done:
    CAPRI_NEXT_TABLE_READ_OFFSET_e(2, TABLE_LOCK_DIS,
                        tcp_tx_process_read_xmit_start,
                        k.p4_txdma_intr_qstate_addr,
                        TCP_TCB_XMIT_OFFSET, TABLE_SIZE_512_BITS)
    nop

clean_retx_drop:
    /*
     * Ring doorbell to set CI
     */
    phvwri          p.app_header_table0_valid, 0;
    phvwri.e        p.p4_intr_global_drop, 1
    nop



/******************************************************************************
 * tcp_tx_retx_timer_expired
 *****************************************************************************/
tcp_tx_retx_timer_expired:
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_DONT_START_RETX_TIMER, TCP_TX_DDOL_DONT_START_RETX_TIMER
    phvwri.c1       p.common_phv_debug_dol_dont_start_retx_timer, 1

    phvwr           p.common_phv_pending_rto, 1
    phvwr           p.t0_s2s_rto_pi, d.{pi_3}.hx
    seq             c1, d.{ci_3}.hx, d.{pi_3}.hx
    tblwr.f         d.{ci_3}.hx, d.{pi_3}.hx
    /*
     * Ring doorbell to set CI
     */

    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    // data will be in r3
    CAPRI_RING_DOORBELL_DATA_NOP(k.p4_txdma_intr_qid, TCP_SCHED_RING_RTO)
    memwr.dx        r4, r3


    // This table need not be locked since it is read-only.
    // Moreover it should not be locked to prevent the bypass
    // cache from being used (bypass cache cannot be used,
    // since the contents are written from rx pipeline)
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS,
                        tcp_tx_read_rx2tx_shared_extra_stage1_start,
                        k.p4_txdma_intr_qstate_addr,
                        TCP_TCB_RX2TX_SHARED_EXTRA_OFFSET, TABLE_SIZE_512_BITS)
    phvwr           p.to_s6_rcv_nxt, d.rcv_nxt
    phvwrpair       p.common_phv_fid, k.p4_txdma_intr_qid, \
                        p.common_phv_qstate_addr, k.p4_txdma_intr_qstate_addr
    phvwr           p.t0_s2s_state, d.state

    b.c1            tcp_tx_rx2tx_end
    /*
     * Launch sesq entry ready with RETX CI as index
     */
    add             r3, d.{sesq_base}.wx, d.sesq_retx_ci, NIC_SESQ_ENTRY_SIZE_SHIFT
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, tcp_tx_sesq_read_ci_stage1_start,
                     r3, TABLE_SIZE_64_BITS)

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
 * tcp_tx_fast_retrans
 *****************************************************************************/
tcp_tx_fast_retrans:
#ifndef HW
    smeqb           c1, d.debug_dol_tx, TCP_TX_DDOL_DONT_START_RETX_TIMER, TCP_TX_DDOL_DONT_START_RETX_TIMER
    phvwri.c1       p.common_phv_debug_dol_dont_start_retx_timer, 1
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
    phvwr           p.to_s6_rcv_nxt, d.rcv_nxt
    phvwrpair       p.common_phv_fid, k.p4_txdma_intr_qid, \
                        p.common_phv_qstate_addr, k.p4_txdma_intr_qstate_addr
    phvwr           p.t0_s2s_state, d.state

    /*
     * Ring doorbell to set CI
     */

    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    // data will be in r3
    CAPRI_RING_DOORBELL_DATA_NOP(k.p4_txdma_intr_qid, TCP_SCHED_RING_FAST_RETRANS)
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
    CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, TCP_SCHED_RING_DELACK_TIMER, r5)
    memwr.dx        r4, r3
    tbladd          d.{ci_2}.hx, 1

    // This table need not be locked since it is read-only.
    // Moreover it should not be locked to prevent the bypass
    // cache from being used (bypass cache cannot be used,
    // since the contents are written from rx pipeline)
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS,
                        tcp_tx_read_rx2tx_shared_extra_stage1_start,
                        k.p4_txdma_intr_qstate_addr,
                        TCP_TCB_RX2TX_SHARED_EXTRA_OFFSET, TABLE_SIZE_512_BITS)
    phvwr           p.to_s6_rcv_nxt, d.rcv_nxt
    phvwrpair       p.common_phv_fid, k.p4_txdma_intr_qid, \
                        p.common_phv_qstate_addr, k.p4_txdma_intr_qstate_addr
    phvwr           p.t0_s2s_state, d.state
    nop.e
    nop
    

/******************************************************************************
 * tcp_tx_rx2tx_abort
 *****************************************************************************/
tcp_tx_rx2tx_abort:
    phvwri          p.p4_intr_global_drop, 1
    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA_NOP(k.p4_txdma_intr_qid, TCP_SCHED_RING_SEND_ACK)
    memwr.dx        r4, r3

tcp_tx_rx2tx_end:
    nop.e
    CAPRI_CLEAR_TABLE_VALID(0)


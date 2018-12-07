/*
 *    Implements the RX stage of the RxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tls_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s6_t1_tcp_rx_k.h"

struct phv_ p;
struct s6_t1_tcp_rx_k_ k;
struct s6_t0_tcp_rx_write_serq_d d;

%%
    .param          tcp_rx_stats_stage_start
    .align

    /*
     * Global conditional variables
     * c7 drop packet and don't send to arm
     */
tcp_rx_write_serq_stage_start2:
    CAPRI_CLEAR_TABLE_VALID(1)

    seq         c1, k.common_phv_pending_txdma, 0
    bcf         [c1], tcp_write_serq2_done
    nop

dma_cmd_write_rx2tx_extra_shared:
    /*
     * DMA rx2tx shared extra
     */
    add         r5, TCP_TCB_RX2TX_SHARED_EXTRA_OFFSET, k.common_phv_qstate_addr
    CAPRI_DMA_CMD_PHV2MEM_SETUP(rx2tx_extra_dma_dma_cmd, r5, rx2tx_extra_rcv_nxt, rx2tx_extra__padding)

#ifndef HW
    /*
     * debug dol (SERQ) cases only
     * If not informing TLS or sending ack, set EOP earlier
     */
    smeqb       c1, k.common_phv_debug_dol, TCP_DDOL_PKT_TO_SERQ, TCP_DDOL_PKT_TO_SERQ
    smeqb.!c1   c1, k.common_phv_debug_dol, TCP_DDOL_DONT_QUEUE_TO_SERQ, TCP_DDOL_DONT_QUEUE_TO_SERQ
    smeqb.c1    c1, k.common_phv_debug_dol, TCP_DDOL_DONT_SEND_ACK, TCP_DDOL_DONT_SEND_ACK
    phvwri.c1   p.rx2tx_extra_dma_dma_cmd_eop, 1
#endif

dma_cmd_write_rx2tx_extra_shared_end:
    // If delayed ack is the only pending work, there are no doorbells
    seq         c1, k.common_phv_pending_txdma, TCP_PENDING_TXDMA_DEL_ACK
    b.c1        tcp_write_serq2_done
    nop

/*
 * Fast retrans can be done without requiring explicit ordering, so
 * use memwr instead of DMA to save a dma cmd in PHV
 */
rx2tx_fast_retrans_ring:
    bbeq        k.common_phv_pending_txdma[TCP_PENDING_TXDMA_FAST_RETRANS_BIT], 0, rx2tx_fast_retrans_done
    addi        r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_SET,
                        DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    tbladd      d.rx2tx_fast_retx_pi, 1
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA(0, k.common_phv_fid,
                        TCP_SCHED_RING_FAST_RETRANS, d.rx2tx_fast_retx_pi)
    memwr.dx        r4, r3
rx2tx_fast_retrans_done:

dma_cmd_ring_tcp_tx_doorbell:
#ifndef HW
    /*
     * Check if we have pending_txdma work, exit if not
     */
    smeqb       c1, k.common_phv_debug_dol, TCP_DDOL_DONT_SEND_ACK, TCP_DDOL_DONT_SEND_ACK
    smeqb       c2, k.common_phv_pending_txdma, TCP_PENDING_TXDMA_ACK_SEND, TCP_PENDING_TXDMA_ACK_SEND
    bcf         [c1 & c2], tcp_write_serq2_done
    nop
#endif

    smeqb       c1, k.common_phv_pending_txdma, TCP_PENDING_TXDMA_ACK_SEND, TCP_PENDING_TXDMA_ACK_SEND
    smeqb       c2, k.common_phv_pending_txdma, TCP_PENDING_TXDMA_SND_UNA_UPDATE, TCP_PENDING_TXDMA_SND_UNA_UPDATE
    bcf         [c1 & c2], rx2tx_send_ack_and_clean_retx_ring
    nop
    b.c1        rx2tx_send_ack_ring
    b.c2        rx2tx_clean_retx_ring
    nop
rx2tx_send_ack_ring:
    tbladd.f    d.rx2tx_send_ack_pi, 1
    CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI(tx_doorbell_dma_cmd, LIF_TCP, 0, k.common_phv_fid,
                                TCP_SCHED_RING_SEND_ACK, d.rx2tx_send_ack_pi, db_data2_pid, db_data2_index)
    b           rx2tx_ring_done
    nop
rx2tx_clean_retx_ring:
    tbladd.f    d.rx2tx_clean_retx_pi, 1
    CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI(tx_doorbell_dma_cmd, LIF_TCP, 0, k.common_phv_fid,
                                TCP_SCHED_RING_CLEAN_RETX, d.rx2tx_clean_retx_pi, db_data2_pid, db_data2_index)
    b           rx2tx_ring_done
    nop
rx2tx_send_ack_and_clean_retx_ring:
    tbladd      d.rx2tx_send_ack_pi, 1
    tbladd.f    d.rx2tx_clean_retx_pi, 1
    CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI(tx_doorbell1_dma_cmd, LIF_TCP, 0, k.common_phv_fid,
                                TCP_SCHED_RING_SEND_ACK, d.rx2tx_send_ack_pi, db_data2_pid, db_data2_index)
    phvwr       p.tx_doorbell1_dma_cmd_wr_fence, 1
    CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI(tx_doorbell_dma_cmd, LIF_TCP, 0, k.common_phv_fid,
                                TCP_SCHED_RING_CLEAN_RETX, d.rx2tx_clean_retx_pi, db_data3_pid, db_data3_index)
rx2tx_ring_done:
    phvwr       p.tx_doorbell_dma_cmd_wr_fence, 1

    seq         c1, k.common_phv_skip_pkt_dma, 1
    bcf         [c1], tx_doorbell_set_eop

    /*
     * c7 is drop case, we want to set EOP on tx doorbell and exit after that
     */
    seq         c1, k.common_phv_write_arq, 1
    seq         c2, k.common_phv_write_serq, 1
    setcf       c7, [!c1 & !c2]

    smeqb       c2, k.common_phv_debug_dol, TCP_DDOL_DONT_QUEUE_TO_SERQ, TCP_DDOL_DONT_QUEUE_TO_SERQ
    smeqb       c3, k.common_phv_debug_dol, TCP_DDOL_PKT_TO_SERQ, TCP_DDOL_PKT_TO_SERQ

    bcf         [c7 | c2 | c3], tx_doorbell_set_eop

    seq         c2, k.common_phv_write_serq, 1
    bcf         [c2], tcp_write_serq2_done
    nop

tx_doorbell_set_eop:
    phvwri     p.tx_doorbell_dma_cmd_eop, 1
    phvwri     p.tx_doorbell_dma_cmd_wr_fence, 1

tcp_write_serq2_done:
    nop.e
    nop

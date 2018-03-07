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

dma_cmd_write_rx2tx_shared:
    /*
     * DMA rx2tx shared
     */
    add         r5, TCP_TCB_RX2TX_SHARED_WRITE_OFFSET, k.common_phv_qstate_addr
    CAPRI_DMA_CMD_PHV2MEM_SETUP(rx2tx_or_cpu_hdr_dma_dma_cmd, r5, rx2tx_snd_una, rx2tx_pad1_rx2tx)

dma_cmd_write_rx2tx_extra_shared:
    /*
     * DMA rx2tx shared extra
     */
    add         r5, TCP_TCB_RX2TX_SHARED_EXTRA_OFFSET, k.common_phv_qstate_addr
    CAPRI_DMA_CMD_PHV2MEM_SETUP(rx2tx_extra_dma_dma_cmd, r5, rx2tx_extra_ato_deadline, rx2tx_extra__padding)

    smeqb       c1, k.common_phv_debug_dol, TCP_DDOL_PKT_TO_SERQ, TCP_DDOL_PKT_TO_SERQ
    smeqb       c2, k.common_phv_debug_dol, TCP_DDOL_DONT_QUEUE_TO_SERQ, TCP_DDOL_DONT_QUEUE_TO_SERQ
    bcf         [!c1 & !c2], dma_cmd_write_rx2tx_extra_shared_end

    /*
     * Reached here on debug dol (SERQ) cases
     * DEBUG only
     */
    smeqb       c1, k.common_phv_debug_dol, TCP_DDOL_DEL_ACK_TIMER, TCP_DDOL_DEL_ACK_TIMER
    smeqb       c2, k.common_phv_debug_dol, TCP_DDOL_DONT_SEND_ACK, TCP_DDOL_DONT_SEND_ACK
    seq         c4, k.common_phv_l7_proxy_en, 1

    // dont start timer, dont send ack and no proxy
    setcf       c3, [!c1 & c2 & !c4]
    phvwri.c3   p.rx2tx_extra_dma_dma_cmd_eop, 1

dma_cmd_write_rx2tx_extra_shared_end:
    /*
     * Currently we seem to be handling either delayed ack or ringing tx
     * doorbell to send the ack, but in the case where we want to send
     * delayed ack and also clean up retx queue (snd_una update) we need
     * to do both
     */
    smeqb       c1, k.common_phv_debug_dol, TCP_DDOL_DEL_ACK_TIMER, TCP_DDOL_DEL_ACK_TIMER
    seq         c2, k.common_phv_pending_del_ack_send, 1
    bcf         [c1 | c2], dma_cmd_start_del_ack_timer
    nop
    // dummy instruction to release the lock
    tblwr.f     d.curr_ts, d.curr_ts

dma_cmd_ring_tcp_tx_doorbell:
    /*
     * Check if we have pending_txdma work, exit if not
     */
    smeqb       c1, k.common_phv_debug_dol, TCP_DDOL_DONT_SEND_ACK, TCP_DDOL_DONT_SEND_ACK
    smeqb       c2, k.common_phv_pending_txdma, TCP_PENDING_TXDMA_SND_UNA_UPDATE, TCP_PENDING_TXDMA_SND_UNA_UPDATE
    bcf         [c1 & !c2], tcp_write_serq2_done
    nop

    CAPRI_DMA_CMD_RING_DOORBELL2_INC_PI(tx_doorbell_or_timer_dma_cmd, LIF_TCP, 0, k.common_phv_fid,
                                TCP_SCHED_RING_PENDING_RX2TX, db_data2_pid, db_data2_index)

    seq         c1, k.common_phv_skip_pkt_dma, 1
    bcf         [c1], tx_doorbell_set_eop

    seq         c1, k.common_phv_l7_proxy_en, 1
    bcf         [c1], tcp_write_serq2_done

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
    phvwri     p.tx_doorbell_or_timer_dma_cmd_eop, 1
    phvwri     p.tx_doorbell_or_timer_dma_cmd_wr_fence, 1
    b           tcp_write_serq2_done
    nop

dma_cmd_start_del_ack_timer:
    tbladd.f    d.ft_pi, 1
    phvwri      p.tx_doorbell_or_timer_dma_cmd_addr, CAPRI_FAST_TIMER_ADDR(LIF_TCP)
    // result will be in r3
    CAPRI_OPERAND_DEBUG(k.s6_t1_s2s_ato)
    CAPRI_TIMER_DATA(0, k.common_phv_fid, TCP_SCHED_RING_FT, k.s6_t1_s2s_ato)
    phvwr       p.{db_data2_pid...db_data2_index}, r3.dx
    phvwri      p.{tx_doorbell_or_timer_dma_cmd_phv_end_addr...tx_doorbell_or_timer_dma_cmd_type}, \
                    ((CAPRI_PHV_END_OFFSET(db_data2_index) << 18) | \
                     (CAPRI_PHV_START_OFFSET(db_data2_pid) << 8) | \
                     CAPRI_DMA_COMMAND_PHV_TO_MEM)

    smeqb       c1, k.common_phv_debug_dol, TCP_DDOL_DONT_QUEUE_TO_SERQ, TCP_DDOL_DONT_QUEUE_TO_SERQ
    phvwri.c1   p.tx_doorbell_or_timer_dma_cmd_eop, 1

    phvwr.f     p.rx2tx_ft_pi, d.ft_pi

tcp_write_serq2_done:
    nop.e
    nop

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
#include "INGRESS_s6_t0_tcp_rx_k.h"

struct phv_ p;
struct s6_t0_tcp_rx_k_ k;
struct s6_t0_tcp_rx_write_serq_d d;

%%
    .param          tcp_slow_rx_write_serq_stage_start
    .align

    /*
     * Global conditional variables
     * c7 drop packet and don't send to arm
     */
tcp_rx_write_serq_stage_start:
    seq         c1, k.common_phv_write_arq, 1
    seq         c2, k.common_phv_write_serq, 0
    seq         c3, k.common_phv_l7_proxy_en, 1
    seq         c4, k.common_phv_skip_pkt_dma, 1
    seq         c5, k.common_phv_fatal_error, 1
    and         r1, k.common_phv_debug_dol, \
                    ~(TCP_DDOL_DONT_SEND_ACK | \
                    TCP_DDOL_BYPASS_BARCO)
    sne         c6, r1, 0
    seq         c7, k.to_s6_payload_len, 0
    setcf       c1, [c1 | c2 | c3 | c4 | c5 | c6 | c7]
    seq         c2, k.common_phv_ooo_rcv, 1
    bcf         [c1 | c2], write_serq_check_more_slowly
    nop

dma_cmd_data:
    phvwri      p.p4_rxdma_intr_dma_cmd_ptr, TCP_PHV_RXDMA_COMMANDS_START

    /* Set the DMA_WRITE CMD for data */
    add         r1, k.to_s6_page, k.to_s6_ooo_offset
    addi        r3, r1, (NIC_PAGE_HDR_SIZE + NIC_PAGE_HEADROOM)

    /*
     * For SACK case, to_s6_payload_len is total accumulated length of
     * packet. s6_s2s_payload_len is the packet we just got and that
     * is what we want to DMA
     */
    CAPRI_DMA_CMD_PKT2MEM_SETUP(pkt_dma_dma_cmd, r3, k.s6_s2s_payload_len)
    seq         c1, k.common_phv_pending_txdma, 0

dma_cmd_descr:
    /* Set the DMA_WRITE CMD for descr */
    add         r1, k.to_s6_descr, NIC_DESC_ENTRY_0_OFFSET

    addi        r3, r0, (NIC_PAGE_HDR_SIZE + NIC_PAGE_HEADROOM)
    phvwr       p.aol_A0, k.{to_s6_page}.dx
    phvwr       p.aol_O0, r3.wx
    phvwr       p.aol_L0, k.{to_s6_payload_len}.wx

    CAPRI_DMA_CMD_PHV2MEM_SETUP(pkt_descr_dma_dma_cmd, r1, aol_A0, aol_next_pkt)

dma_tcp_flags:
    add         r1, k.to_s6_descr, NIC_DESC_ENTRY_TCP_FLAGS_OFFSET
    CAPRI_DMA_CMD_PHV2MEM_SETUP(tcp_flags_dma_dma_cmd, r1, tcp_app_header_flags, tcp_app_header_flags)

dma_cmd_serq_slot:
    CAPRI_OPERAND_DEBUG(k.to_s6_serq_pidx)
    sll         r5, k.to_s6_serq_pidx, NIC_SERQ_ENTRY_SIZE_SHIFT
    /* Set the DMA_WRITE CMD for SERQ slot */
    add         r1, r5, d.serq_base
    // increment serq pi as a part of ringing dorrbell

    phvwr       p.ring_entry_descr_addr, k.to_s6_descr
    /* The new SERQ defintion includes the descriptor pointer followed by the first AOL */
    CAPRI_DMA_CMD_PHV2MEM_SETUP(ring_slot_dma_cmd, r1, ring_entry_descr_addr, aol_L0)

ring_doorbell:
    add         r1, k.to_s6_serq_pidx, 1
    CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI_STOP_FENCE(tls_doorbell_dma_cmd, LIF_TLS, 0,
                                 k.common_phv_fid, TLS_SCHED_RING_SERQ,
                                 r1, db_data_pid, db_data_index)
    nop.e
    nop

write_serq_check_more_slowly:
    j           tcp_slow_rx_write_serq_stage_start
    nop


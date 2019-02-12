/*
 *  Implements the RX stage of the RxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s5_t2_tcp_rx_k.h"

struct phv_ p;
struct s5_t2_tcp_rx_k_ k;
struct s5_t2_tcp_rx_ooo_qbase_cb_load_d d;

%%
    .param          tcp_rx_write_ooq_stage_start
    .align

tcp_ooo_qbase_cb_load:
    CAPRI_CLEAR_TABLE_VALID(2)

    add             r1, k.t2_s2s_ooo_queue_id, r0
    .brbegin
    br r1[1:0]
    nop
    .brcase 0
        seq         c1, d.ooo_qbase_addr0, r0
        tblwr.c1    d.ooo_qbase_addr0, k.t2_s2s_ooo_qbase_addr
        phvwr.!c1   p.t2_s2s_ooo_qbase_addr, d.ooo_qbase_addr0
        b           tcp_ooo_launch_dma
        nop
    .brcase 1
        seq         c1, d.ooo_qbase_addr1, r0
        tblwr.c1    d.ooo_qbase_addr1, k.t2_s2s_ooo_qbase_addr
        phvwr.!c1   p.t2_s2s_ooo_qbase_addr, d.ooo_qbase_addr1
        b           tcp_ooo_launch_dma
        nop
    .brcase 2
        seq         c1, d.ooo_qbase_addr2, r0
        tblwr.c1    d.ooo_qbase_addr2, k.t2_s2s_ooo_qbase_addr
        phvwr.!c1   p.t2_s2s_ooo_qbase_addr, d.ooo_qbase_addr2
        b           tcp_ooo_launch_dma
        nop
    .brcase 3
        seq         c1, d.ooo_qbase_addr3, r0
        tblwr.c1    d.ooo_qbase_addr3, k.t2_s2s_ooo_qbase_addr
        phvwr.!c1   p.t2_s2s_ooo_qbase_addr, d.ooo_qbase_addr3
        b           tcp_ooo_launch_dma
        nop
    .brend
tcp_ooo_launch_dma:
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(2, tcp_rx_write_ooq_stage_start)

#if 0
    add             r2, k.t2_s2s_ooo_rx2tx_ready_qid, r0
    .brbegin
    br r2[2:0]
    nop
    .brcase 0
        nop
    .brcase 1
        add         r2, d.ooo_qbase_addr0, r0
        phvwr       p.ooq_rx2tx_queue_entry_ooq_rx2tx_qentry_addr, r2
        CAPRI_DMA_CMD_PHV2MEM_SETUP(ooo_rx2tx_ring_slot_dma_cmd, r2, ooq_rx2tx_queue_entry_ooq_rx2tx_qentry_addr, ooq_rx2tx_queue_entry_ooq_rx2tx_num_entries)
        add         r3, d.ooo_qbase_pi, 1
        CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI_STOP_FENCE(ooq_rx2tx_doorbell_dma_cmd, LIF_TCP, TCP_OOO_RX2TX_QTYPE,
                                 k.common_phv_fid, TCP_OOO_RX2TX_QTYPE_RING0,
                                 r3, db_data_pid, db_data_index)
        tblwr       d.ooo_qbase_pi, r3

    .brcase 2
        add         r2, d.ooo_qbase_addr1, r0
        phvwr       p.ooq_rx2tx_queue_entry_ooq_rx2tx_qentry_addr, r2
        CAPRI_DMA_CMD_PHV2MEM_SETUP(ooo_rx2tx_ring_slot_dma_cmd, r2, ooq_rx2tx_queue_entry_ooq_rx2tx_qentry_addr, ooq_rx2tx_queue_entry_ooq_rx2tx_num_entries)
        add         r3, d.ooo_qbase_pi, 1
        CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI_STOP_FENCE(ooq_rx2tx_doorbell_dma_cmd, LIF_TCP, TCP_OOO_RX2TX_QTYPE,
                                 k.common_phv_fid, TCP_OOO_RX2TX_QTYPE_RING0,
                                 r3, db_data_pid, db_data_index)
        tblwr       d.ooo_qbase_pi, r3
    .brcase 3
        add         r2, d.ooo_qbase_addr2, r0
        phvwr       p.ooq_rx2tx_queue_entry_ooq_rx2tx_qentry_addr, r2
        CAPRI_DMA_CMD_PHV2MEM_SETUP(ooo_rx2tx_ring_slot_dma_cmd, r2, ooq_rx2tx_queue_entry_ooq_rx2tx_qentry_addr, ooq_rx2tx_queue_entry_ooq_rx2tx_num_entries)
        add         r3, d.ooo_qbase_pi, 1
        CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI_STOP_FENCE(ooq_rx2tx_doorbell_dma_cmd, LIF_TCP, TCP_OOO_RX2TX_QTYPE,
                                 k.common_phv_fid, TCP_OOO_RX2TX_QTYPE_RING0,
                                 r3, db_data_pid, db_data_index)
        tblwr       d.ooo_qbase_pi, r3
    .brcase 4
        add         r2, d.ooo_qbase_addr3, r0
        phvwr       p.ooq_rx2tx_queue_entry_ooq_rx2tx_qentry_addr, r2
        CAPRI_DMA_CMD_PHV2MEM_SETUP(ooo_rx2tx_ring_slot_dma_cmd, r2, ooq_rx2tx_queue_entry_ooq_rx2tx_qentry_addr, ooq_rx2tx_queue_entry_ooq_rx2tx_num_entries)
        add         r3, d.ooo_qbase_pi, 1
        CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI_STOP_FENCE(ooq_rx2tx_doorbell_dma_cmd, LIF_TCP, TCP_OOO_RX2TX_QTYPE,
                                 k.common_phv_fid, TCP_OOO_RX2TX_QTYPE_RING0,
                                 r3, db_data_pid, db_data_index)
        tblwr       d.ooo_qbase_pi, r3
    .brcase 5
        nop
    .brcase 6
        nop
    .brcase 7
        nop
    .brend
    //add             r1, k.t2_s2s_ooo_pkt_descr_addr, r0
    CAPRI_DMA_CMD_PHV2MEM_SETUP(ooq_tcp_flags_dma_cmd, r1, tcp_app_header_p4plus_app_id, tcp_app_header_prev_echo_ts)
#endif
    nop.e
    nop

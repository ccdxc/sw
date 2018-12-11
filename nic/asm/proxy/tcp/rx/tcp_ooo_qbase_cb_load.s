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
#include "INGRESS_s6_t3_tcp_rx_k.h"

struct phv_ p;
struct s6_t3_tcp_rx_k_ k;
struct s6_t3_tcp_rx_d d;

%%
    .align
tcp_ooo_qbase_cb_load:
    add r1, k.to_s6_ooo_queue_id, r0
    .brbegin
    br r1[1:0]
    nop
    .brcase 0
        seq c1, d.u.tcp_ooo_qbase_cb_load_d.ooo_qbase_addr0, r0
        tblwr.c1 d.u.tcp_ooo_qbase_cb_load_d.ooo_qbase_addr0, k.s6_t3_s2s_ooo_qbase_addr
        sll r2, k.to_s6_ooo_tail_index, 3
        add r1, d.u.tcp_ooo_qbase_cb_load_d.ooo_qbase_addr0, r2
        // this addr in r1 will be used for DMA'ing the rnmdr
    .brcase 1 
        seq c1, d.u.tcp_ooo_qbase_cb_load_d.ooo_qbase_addr1, r0
        tblwr.c1 d.u.tcp_ooo_qbase_cb_load_d.ooo_qbase_addr1, k.s6_t3_s2s_ooo_qbase_addr
        sll r2, k.to_s6_ooo_tail_index, 3
        add r1, d.u.tcp_ooo_qbase_cb_load_d.ooo_qbase_addr1, r2
        // this addr in r1 will be used for DMA'ing the rnmdr
    .brcase 2 
        seq c1, d.u.tcp_ooo_qbase_cb_load_d.ooo_qbase_addr2, r0
        tblwr.c1 d.u.tcp_ooo_qbase_cb_load_d.ooo_qbase_addr2, k.s6_t3_s2s_ooo_qbase_addr
        sll r2, k.to_s6_ooo_tail_index, 3
        add r1, d.u.tcp_ooo_qbase_cb_load_d.ooo_qbase_addr2, r2
        // this addr in r1 will be used for DMA'ing the rnmdr
    .brcase 3
        seq c1, d.u.tcp_ooo_qbase_cb_load_d.ooo_qbase_addr3, r0
        tblwr.c1 d.u.tcp_ooo_qbase_cb_load_d.ooo_qbase_addr3, k.s6_t3_s2s_ooo_qbase_addr
        sll r2, k.to_s6_ooo_tail_index, 3
        add r1, d.u.tcp_ooo_qbase_cb_load_d.ooo_qbase_addr3, r2
        // this addr in r1 will be used for DMA'ing the rnmdr 
    .brend 
    CAPRI_DMA_CMD_PHV2MEM_SETUP(ooo_slot_addr_dma_cmd, r1, ooq_slot_addr_ooq_slot_addr, ooq_slot_addr_ooq_slot_addr)

    add r2, k.to_s6_ooo_rx2tx_ready_qid, r0
    .brbegin
    br r2[2:0]
    nop
    .brcase 0
        nop 
    .brcase 1
        add r2, d.u.tcp_ooo_qbase_cb_load_d.ooo_qbase_addr0, r0
        phvwr p.ooq_rx2tx_queue_entry_ooq_rx2tx_qentry_addr, r2 
        CAPRI_DMA_CMD_PHV2MEM_SETUP(ooo_rx2tx_ring_slot_dma_cmd, r2, ooq_rx2tx_queue_entry_ooq_rx2tx_qentry_addr, ooq_rx2tx_queue_entry_ooq_rx2tx_num_entries)
        add r3, d.u.tcp_ooo_qbase_cb_load_d.ooo_qbase_pi, 1
        CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI_STOP_FENCE(ooq_rx2tx_doorbell_dma_cmd, LIF_TCP, TCP_OOO_RX2TX_QTYPE,
                                 k.common_phv_fid, TCP_OOO_RX2TX_QTYPE_RING0,
                                 r3, db_data_pid, db_data_index)
        tblwr d.u.tcp_ooo_qbase_cb_load_d.ooo_qbase_pi, r3

    .brcase 2
        add r2, d.u.tcp_ooo_qbase_cb_load_d.ooo_qbase_addr1, r0
        phvwr p.ooq_rx2tx_queue_entry_ooq_rx2tx_qentry_addr, r2 
        CAPRI_DMA_CMD_PHV2MEM_SETUP(ooo_rx2tx_ring_slot_dma_cmd, r2, ooq_rx2tx_queue_entry_ooq_rx2tx_qentry_addr, ooq_rx2tx_queue_entry_ooq_rx2tx_num_entries)
        add r3, d.u.tcp_ooo_qbase_cb_load_d.ooo_qbase_pi, 1
        CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI_STOP_FENCE(ooq_rx2tx_doorbell_dma_cmd, LIF_TCP, TCP_OOO_RX2TX_QTYPE,
                                 k.common_phv_fid, TCP_OOO_RX2TX_QTYPE_RING0,
                                 r3, db_data_pid, db_data_index)
        tblwr d.u.tcp_ooo_qbase_cb_load_d.ooo_qbase_pi, r3
    .brcase 3
        add r2, d.u.tcp_ooo_qbase_cb_load_d.ooo_qbase_addr2, r0
        phvwr p.ooq_rx2tx_queue_entry_ooq_rx2tx_qentry_addr, r2 
        CAPRI_DMA_CMD_PHV2MEM_SETUP(ooo_rx2tx_ring_slot_dma_cmd, r2, ooq_rx2tx_queue_entry_ooq_rx2tx_qentry_addr, ooq_rx2tx_queue_entry_ooq_rx2tx_num_entries)
        add r3, d.u.tcp_ooo_qbase_cb_load_d.ooo_qbase_pi, 1
        CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI_STOP_FENCE(ooq_rx2tx_doorbell_dma_cmd, LIF_TCP, TCP_OOO_RX2TX_QTYPE,
                                 k.common_phv_fid, TCP_OOO_RX2TX_QTYPE_RING0,
                                 r3, db_data_pid, db_data_index)
        tblwr d.u.tcp_ooo_qbase_cb_load_d.ooo_qbase_pi, r3
    .brcase 4
        add r2, d.u.tcp_ooo_qbase_cb_load_d.ooo_qbase_addr3, r0
        phvwr p.ooq_rx2tx_queue_entry_ooq_rx2tx_qentry_addr, r2 
        CAPRI_DMA_CMD_PHV2MEM_SETUP(ooo_rx2tx_ring_slot_dma_cmd, r2, ooq_rx2tx_queue_entry_ooq_rx2tx_qentry_addr, ooq_rx2tx_queue_entry_ooq_rx2tx_num_entries)
        add r3, d.u.tcp_ooo_qbase_cb_load_d.ooo_qbase_pi, 1
        CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI_STOP_FENCE(ooq_rx2tx_doorbell_dma_cmd, LIF_TCP, TCP_OOO_RX2TX_QTYPE,
                                 k.common_phv_fid, TCP_OOO_RX2TX_QTYPE_RING0,
                                 r3, db_data_pid, db_data_index)
        tblwr d.u.tcp_ooo_qbase_cb_load_d.ooo_qbase_pi, r3
    .brcase 5
        nop 
    .brcase 6
        nop 
    .brcase 7
        nop 
    .brend
  
    nop.e
    nop
           
      

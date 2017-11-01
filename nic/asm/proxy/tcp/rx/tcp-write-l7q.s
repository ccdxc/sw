/*
 *    Implements the RX stage of the RxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct phv_ p;
struct tcp_rx_write_l7q_k k;
struct tcp_rx_write_l7q_write_l7q_d d;

%%
    .align

tcp_rx_write_l7q_stage6_start:
    CAPRI_CLEAR_TABLE2_VALID
    //CAPRI_OPERAND_DEBUG(k.s6_s2s_debug_stage0_3_thread)
    //CAPRI_OPERAND_DEBUG(k.s6_s2s_debug_stage4_7_thread)
    //tblwr       d.debug_stage0_3_thread, k.s6_s2s_debug_stage0_3_thread
    //tblwr       d.debug_stage4_7_thread, k.s6_s2s_debug_stage4_7_thread
    sne         c1, k.common_phv_l7_proxy_en, r0
    seq         c2, k.to_s6_payload_len, r0
    bcf         [!c1 | c2], flow_write_l7q_process_done
    nop

dma_cmd_l7_descr:
    /* Set the DMA_WRITE CMD for descr */
    add         r5, k.{s6_t2_s2s_l7_descr_sbit0_ebit15...s6_t2_s2s_l7_descr_sbit16_ebit31}, r0
    addi        r1, r5, NIC_DESC_ENTRY_0_OFFSET

    phvwr       p.aol_A1, k.{to_s6_page}.dx
    addi        r3, r0, (NIC_PAGE_HDR_SIZE + NIC_PAGE_HEADROOM)
    phvwr       p.aol_O1, r3.wx
    phvwr       p.aol_L1, k.{to_s6_payload_len}.wx
    phvwr       p.aol_A2, r0
    phvwr       p.aol_O2, r0
    phvwr       p.aol_L2, r0
    phvwr       p.aol_next_addr, r0

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd7_dma_cmd, r1, aol_A1, aol_next_pkt)
    addi        r7, r0, 1
    /*
    smeqb       c1, k.common_phv_debug_dol, TCP_DDOL_DONT_QUEUE_TO_SERQ, TCP_DDOL_DONT_QUEUE_TO_SERQ
    bcf         [c1], dma_cmd_write_rx2tx_shared
    nop
    */
dma_cmd_l7q_slot:
    CAPRI_OPERAND_DEBUG(d.{l7q_pidx}.hx)
    sll         r5, d.{l7q_pidx}.hx, NIC_L7Q_ENTRY_SIZE_SHIFT
    /* Set the DMA_WRITE CMD for L7 Queue slot */
    add         r1, r5, d.{l7q_base}.wx

    // increment pi as a part of ringing dorrbell
    phvwr       p.l7_ring_entry_descr_addr, k.{s6_t2_s2s_l7_descr_sbit0_ebit15...s6_t2_s2s_l7_descr_sbit16_ebit31}
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd8_dma_cmd, r1, l7_ring_entry_descr_addr, l7_ring_entry_descr_addr)
    addi        r7, r7, 1

tcp_l7q_produce:
    
    smeqb       c1, k.common_phv_debug_dol, TCP_DDOL_PKT_TO_L7Q, TCP_DDOL_PKT_TO_L7Q
    bcf         [!c1], ring_doorbell
    nop
    CAPRI_DMA_CMD_STOP_FENCE(dma_cmd8_dma_cmd)
    b           flow_write_l7q_process_done
    nop
ring_doorbell:

    CAPRI_DMA_CMD_RING_DOORBELL2(dma_cmd9_dma_cmd, 
                                 LIF_APP_REDIR,
                                 0,
                                 k.common_phv_fid,
                                 0,
                                 d.{l7q_pidx}.hx, 
                                 db_data_pid, 
                                 db_data_index)
    tbladd      d.{l7q_pidx}.hx, 1
    CAPRI_DMA_CMD_STOP_FENCE(dma_cmd9_dma_cmd)
    addi        r7, r0, 1

flow_write_l7q_process_done:
    nop.e
    nop

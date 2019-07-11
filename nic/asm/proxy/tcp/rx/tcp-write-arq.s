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
#include "INGRESS_s6_t1_tcp_rx_k.h"

#define TCP_ACTL_MSG_TYPE_PKT   0
#define TCP_ACTL_MSG_TYPE_SHIFT 56

struct phv_ p;
struct s6_t1_tcp_rx_k_ k;
struct s6_t1_tcp_rx_write_arq_d d;

%%
    .align
#ifndef TCP_ACTL_Q
    .param          ARQRX_BASE
#else
    .param          TCP_ACTL_Q_BASE
#endif
tcp_rx_write_arq_stage_start:
    CAPRI_CLEAR_TABLE1_VALID


    /* if (k.write_serq) is set in a previous stage , trigger writes to serq slot */
    sne         c1, k.common_phv_write_arq, r0
    bcf         [!c1], flow_write_arq_process_done
    nop

    phvwri      p.p4_rxdma_intr_dma_cmd_ptr, TCP_PHV_RXDMA_COMMANDS_START

    seq         c1, k.common_phv_write_tcp_app_hdr, 1
    bcf         [!c1], dma_cmd_data
    add         r6, r0, k.to_s6_payload_len

dma_cmd_cpu_hdr:
    addi        r6, r0, NIC_CPU_HDR_SIZE_BYTES
    phvwri      p.cpu_hdr1_src_lif, 0
    addi        r1, r0, LIF_TCP
    phvwr       p.cpu_hdr1_lif, r1.hx
    phvwri      p.cpu_hdr1_qtype, 0
    phvwr       p.cpu_hdr1_qid, k.{common_phv_fid}.wx
    phvwri      p.cpu_hdr1_lkp_dir, 0
    phvwri      p.cpu_hdr1_lkp_inst,0
    phvwri      p.cpu_hdr1_lkp_type,0
    phvwri      p.cpu_hdr1_flags, 0
    phvwri      p.cpu_hdr1_l2_offset, 0xFFFF
    phvwri      p.cpu_hdr1_l3_offset_1, 0xFF
    phvwri      p.cpu_hdr2_l3_offset_2, 0xFF
    phvwri      p.cpu_hdr2_l4_offset, 0xFFFF
    phvwri      p.cpu_hdr2_payload_offset, 0xFFFF
    phvwri      p.cpu_hdr3_tcp_options, 0
    addi        r1, r0, 9000
    phvwr       p.cpu_hdr3_tcp_mss, r1.hx
    phvwri      p.cpu_hdr3_tcp_ws, 0
    add         r1, r0, k.to_s6_page
    addi        r3, r1, (NIC_PAGE_HDR_SIZE + NIC_PAGE_HEADROOM)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(cpu_hdr_dma_dma_cmd, r3, cpu_hdr1_src_lif, cpu_hdr3_tcp_ws)
dma_cmd_data:

    seq         c1, k.to_s6_payload_len, r0
    b.c1        dma_cmd_descr

    /* Set the DMA_WRITE CMD for data */
    add         r1, r0, k.to_s6_page
    addi        r3, r1, (NIC_PAGE_HDR_SIZE + NIC_PAGE_HEADROOM)


    CAPRI_DMA_CMD_PKT2MEM_SETUP(pkt_dma_dma_cmd, r3, k.to_s6_payload_len)

dma_cmd_descr:
    /* Set the DMA_WRITE CMD for descr */
    add         r5, k.to_s6_descr, r0
    addi        r1, r5, PKT_DESC_AOL_OFFSET


    phvwr       p.aol_A0, k.{to_s6_page}.dx

    sub         r4, r3, k.to_s6_page
    phvwr       p.aol_O0, r4.wx

    phvwr       p.aol_L0, r6.wx

    CAPRI_DMA_CMD_PHV2MEM_SETUP(pkt_descr_dma_dma_cmd, r1, aol_A0, aol_next_pkt)    
    
    smeqb       c1, k.common_phv_debug_dol, TCP_DDOL_LEAVE_IN_ARQ, TCP_DDOL_LEAVE_IN_ARQ
dma_cmd_arq_slot:
#ifdef TCP_ACTL_Q
    addui       r4, r0, hiword(TCP_ACTL_Q_BASE)
    addi        r4, r4, loword(TCP_ACTL_Q_BASE)
    TCP_ACTL_Q_BASE_FOR_ID(r2, r4, k.s6_t1_s2s_cpu_id)
    add         r6, r0, d.{tcp_actl_q_pindex}.wx
    addi        r3, r0, TCP_ACTL_MSG_TYPE_PKT
    add         r3, k.to_s6_descr, r3, TCP_ACTL_MSG_TYPE_SHIFT
    TCP_ACTL_Q_ENQUEUE(r5,
                   r3,
                   r6,
                   r2,
                   ring_entry_64_dword,
                   ring_entry_64_dword,
                   ring_slot_dma_cmd,
                   1,
                   1,
                   c1)
#else
    addui       r4, r0, hiword(ARQRX_BASE)
    addi        r4, r4, loword(ARQRX_BASE)
    CPU_RX_ARQ_BASE_FOR_ID(r2, r4, k.s6_t1_s2s_cpu_id)
    add         r6, r0, d.{arq_pindex}.wx 
    CPU_RX_ENQUEUE(r5,
                   k.to_s6_descr,
                   r6,
                   r2,
                   ring_entry_64_dword,
                   ring_entry_64_dword,
                   ring_slot_dma_cmd, 
                   1, 
                   1, 
                   c1)  
#endif
    
flow_write_arq_process_done:
    nop.e
    nop


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
struct tcp_rx_write_arq_k k;
struct tcp_rx_write_arq_write_arq_d d;

%%
    .align    
tcp_rx_write_arq_stage6_start:
    CAPRI_CLEAR_TABLE1_VALID


    /* if (k.write_serq) is set in a previous stage , trigger writes to serq slot */
    sne         c1, k.common_phv_write_arq, r0
    bcf         [!c1], flow_write_arq_process_done
    nop

dma_cmd_data:
    phvwri      p.p4_rxdma_intr_dma_cmd_ptr, TCP_PHV_RXDMA_COMMANDS_START

    /* Set the DMA_WRITE CMD for data */
    add         r1, r0, k.to_s6_page
    addi        r3, r1, (NIC_PAGE_HDR_SIZE + NIC_PAGE_HEADROOM)

    sne         c1, k.common_phv_write_tcp_app_hdr, r0
    bcf         [c1], dma_cmd_cpu_hdr
    nop

#if 1
    addi        r6, r6, (70+NIC_CPU_HDR_SIZE_BYTES)
    CAPRI_DMA_CMD_PKT2MEM_SETUP(dma_cmd0_dma_cmd, r3, r6)
#else
    CAPRI_DMA_CMD_PKT2MEM_SETUP(dma_cmd0_dma_cmd, r3, k.to_s6_payload_len)
#endif
        
     b          dma_cmd_descr
     nop
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
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd1_dma_cmd, r3, cpu_hdr1_src_lif, cpu_hdr3_tcp_ws)
        
dma_cmd_descr:    
    /* Set the DMA_WRITE CMD for descr */
    add         r5, k.to_s6_descr, r0
    addi        r1, r5, PKT_DESC_AOL_OFFSET


    phvwr       p.aol_A0, k.{to_s6_page}.dx

    sub         r4, r3, k.to_s6_page
    phvwr       p.aol_O0, r4.wx

    /* We have to get the length of (eth+ip+tcp) for syn ack from P4.
     * Till then...
     */
#if 1
    add         r4, r6, r0
#else
    add         r4, k.to_s6_payload_len, NIC_CPU_HDR_SIZE_BYTES
#endif
    phvwr       p.aol_L0, r4.wx

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd2_dma_cmd, r1, aol_A0, aol_next_pkt)    
    

    smeqb       c1, k.common_phv_debug_dol, TCP_DDOL_LEAVE_IN_ARQ, TCP_DDOL_LEAVE_IN_ARQ
dma_cmd_arq_slot:
    CPU_ARQ_PIDX_READ_INC(r6, 0, struct tcp_rx_write_arq_write_arq_d, pi_0)
    CPU_RX_ENQUEUE(r5,
                   k.to_s6_descr,
                   r6,
                   k.to_s6_xrq_base,
                   ring_entry_descr_addr,
                   dma_cmd3_dma_cmd, 
                   1, 
                   1, 
                   c1)  

    
flow_write_arq_process_done:
    nop.e
    nop


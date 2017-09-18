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
    
    phvwr       p.dma_cmd0_dma_cmd_addr, r3
    phvwr       p.dma_cmd0_dma_cmd_size, k.to_s6_payload_len
    
    phvwri      p.dma_cmd0_dma_cmd_type, CAPRI_DMA_COMMAND_PKT_TO_MEM
    phvwri      p.dma_cmd0_dma_cmd_eop, 0


dma_cmd_descr:    
    /* Set the DMA_WRITE CMD for descr */
    add         r5, k.to_s6_descr, r0
    addi        r1, r5, NIC_DESC_ENTRY_0_OFFSET
    phvwr       p.dma_cmd1_dma_cmd_addr, r1

    phvwr       p.aol_A0, k.{to_s6_page}.dx
    addi        r3, r0, (NIC_PAGE_HDR_SIZE + NIC_PAGE_HEADROOM)
    phvwr       p.aol_O0, r3.wx
    phvwr       p.aol_L0, k.{to_s6_payload_len}.wx

    phvwri      p.dma_cmd1_dma_cmd_phv_start_addr, TCP_PHV_AOL_DESC_START
    phvwri      p.dma_cmd1_dma_cmd_phv_end_addr, TCP_PHV_AOL_DESC_END

    phvwri      p.dma_cmd1_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri      p.dma_cmd1_dma_cmd_eop, 0
    addi        r7, r0, 1
    

    smeqb       c1, k.common_phv_debug_dol, TCP_DDOL_DONT_QUEUE_TO_ARQ, TCP_DDOL_DONT_QUEUE_TO_ARQ
    bcf         [c1], flow_write_arq_process_done
    nop
dma_cmd_arq_slot:
    CPU_ARQ_PIDX_READ_INC(r6, 0, struct tcp_rx_write_arq_write_arq_d, pi_0)
    CPU_RX_ENQUEUE(r5,
                   k.to_s6_descr,
                   r6,
                   k.to_s6_xrq_base,
                   ring_entry_descr_addr,
                   dma_cmd2_dma_cmd, 
                   1, 
                   1)  

    
flow_write_arq_process_done:
    nop.e
    nop


#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "cpu-table.h"

struct phv_ p;
struct cpu_rx_write_arqrx_k k;
struct cpu_rx_write_arqrx_d d;

%%
    .align
cpu_rx_write_arq_start:

    CAPRI_CLEAR_TABLE0_VALID
	tblwr		d.u.write_arqrx_d.curr_ts, r4

dma_cmd_data:
    addi    r5, r0, CAPRI_PHV_START_OFFSET(dma_cmd0_dma_cmd_type) / 16
    add     r4, r5, r0
    phvwr   p.p4_rxdma_intr_dma_cmd_ptr, r4
    add     r1, r0, k.to_s3_page
    addi    r3, r1, (NIC_PAGE_HDR_SIZE + NIC_PAGE_HEADROOM)
    phvwr   p.dma_cmd0_dma_cmd_addr, r3
    phvwr   p.dma_cmd0_dma_cmd_size, k.to_s3_payload_len 
    phvwri  p.dma_cmd0_dma_cmd_type, CAPRI_DMA_COMMAND_PKT_TO_MEM
    phvwri  p.dma_cmd0_dma_cmd_eop, 0

dma_cmd_descr:
    /* Set teh DMA_WRITE CMD for descr */
    add     r5, k.to_s3_descr, r0
    addi    r1, r5, NIC_DESC_ENTRY_0_OFFSET
    phvwr   p.dma_cmd1_dma_cmd_addr, r1

    phvwr   p.aol_A0, k.{to_s3_page}.dx
    addi    r3, r0, (NIC_PAGE_HDR_SIZE + NIC_PAGE_HEADROOM)
    phvwr   p.aol_O0, r3.wx
    phvwr   p.aol_L0, k.{to_s3_payload_len}.wx

    phvwri  p.dma_cmd1_dma_cmd_phv_start_addr, CPU_PHV_AOL_DESC_START
    phvwri  p.dma_cmd1_dma_cmd_phv_end_addr, CPU_PHV_AOL_DESC_END
    phvwri  p.dma_cmd1_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri  p.dma_cmd1_dma_cmd_eop, 0

dma_cmd_arqrx_slot:
    CPU_RX_ENQUEUE(r5,
                   k.to_s3_descr,
                   k.to_s3_arqrx_pindex,
                   k.to_s3_arqrx_base,
                   ring_entry_descr_addr,
                   dma_cmd2_dma_cmd, 
                   1, 
                   1)  
cpu_write_arqrx_done:
    nop.e
    nop

#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "cpu-table.h"

struct phv_ p;
struct cpu_rx_write_arqrx_k k;
struct cpu_rx_write_arqrx_d d;

%%
    .param ARQRX_BASE
    .align
cpu_rx_write_arq_start:
    // Read and increment ARQ PI first in order to release table lock
    CPU_ARQ_PIDX_READ_INC(r6, k.t0_s2s_arqrx_id, d, u.write_arqrx_d.pi_0, r2, r3)

    CAPRI_CLEAR_TABLE0_VALID
    smeqb   c5, k.common_phv_flags, CPUCB_FLAG_ADD_QS_PKT_TRLR, CPUCB_FLAG_ADD_QS_PKT_TRLR

dma_cmd_data:
    addi    r5, r0, CAPRI_PHV_START_OFFSET(dma_cmd0_dma_cmd_type) / 16
    add     r4, r5, r0
    phvwr   p.p4_rxdma_intr_dma_cmd_ptr, r4
    add     r1, r0, k.t0_s2s_page
    addi    r3, r1, (NIC_PAGE_HDR_SIZE + NIC_PAGE_HEADROOM)
    phvwr   p.dma_cmd0_dma_cmd_addr, r3
    phvwr   p.dma_cmd0_dma_cmd_size, k.t0_s2s_payload_len 
    phvwri  p.dma_cmd0_dma_cmd_type, CAPRI_DMA_COMMAND_PKT_TO_MEM
    phvwri  p.dma_cmd0_dma_cmd_eop, 0

    bcf     [!c5], dma_cmd_descr
    nop

dma_cmd_qs_trailer:
    # get trailer address in HBM after pkt
    add     r3, k.t0_s2s_page, (NIC_PAGE_HDR_SIZE + NIC_PAGE_HEADROOM)
    add     r3, r3, k.t0_s2s_payload_len

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd1_dma_cmd,
                                r3,
                                quiesce_pkt_trlr_timestamp,
                                quiesce_pkt_trlr_timestamp)
dma_cmd_descr:
    /* Set teh DMA_WRITE CMD for descr */
    add     r5, k.t0_s2s_descr, r0
    addi    r1, r5, NIC_DESC_ENTRY_0_OFFSET
    phvwr   p.dma_cmd2_dma_cmd_addr, r1

    phvwr   p.aol_A0, k.{t0_s2s_page}.dx
    addi    r3, r0, (NIC_PAGE_HDR_SIZE + NIC_PAGE_HEADROOM)
    phvwr   p.aol_O0, r3.wx
    add     r3, r0, k.t0_s2s_payload_len
    addi.c5 r3, r3, sizeof(p.quiesce_pkt_trlr_timestamp)
    phvwr   p.aol_L0, r3.wx

    phvwri  p.dma_cmd2_dma_cmd_phv_start_addr, CPU_PHV_AOL_DESC_START
    phvwri  p.dma_cmd2_dma_cmd_phv_end_addr, CPU_PHV_AOL_DESC_END
    phvwri  p.dma_cmd2_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri  p.dma_cmd2_dma_cmd_eop, 0

dma_cmd_arqrx_slot:
    smeqb   c1, k.common_phv_debug_dol, CPU_DDOL_PKT_TO_ARQ, CPU_DDOL_PKT_TO_ARQ
    
    addui      r5, r0, hiword(ARQRX_BASE)
    addi       r5, r5, loword(ARQRX_BASE)
    CPU_RX_ARQ_BASE_FOR_ID(r2, r5, k.t0_s2s_arqrx_id)

    // pindex will be in r6
    CPU_RX_ENQUEUE(r5,
                   k.t0_s2s_descr,
                   r6,
                   r2,
                   ring_entry_descr_addr,
                   dma_cmd3_dma_cmd, 
                   1, 
                   1,
                   c1)  

cpu_write_arqrx_done:
    nop.e
    nop

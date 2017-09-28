#include "INGRESS_p.h"
#include "capri-macros.h"
#include "ingress.h"
#include "defines.h"

struct phv_ p;
struct tx_table_s1_t0_k k;
struct tx_table_s1_t0_adminq_process_desc_d d;

%%

.align
adminq_operation:

    // Setup DMA CMD PTR
    phvwri      p.p4_txdma_intr_dma_cmd_ptr, ADMINQ_DMA_CMD_PTR

    // End of pipeline - Make sure no more tables will be launched
    phvwri      p.{app_header_table0_valid...app_header_table3_valid}, 0

    // Create CQ in PHV
    phvwr       p.adminq_s2s_cmd_status, d.opcode
    phvwri      p.adminq_s2s_cpl_id, 10
    phvwri      p.adminq_s2s_color, 1
    phvwr       p.adminq_s2s_cmd_data0, d.cmd_data0
    phvwr       p.adminq_s2s_cmd_data1, d.cmd_data1
    phvwr       p.adminq_s2s_cmd_data2, d.cmd_data2

    // DMA cap_phv_intr_global_t (18B)
    phvwri      p.dma_cmd0_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    phvwri      p.dma_cmd0_dma_cmd_phv_start_addr, 0
    phvwri      p.dma_cmd0_dma_cmd_phv_end_addr, 16
    phvwri      p.dma_cmd0_dma_pkt_eop, 0
    phvwri      p.dma_cmd0_dma_cmd_eop, 0

    // DMA completion to Host Memory
    phvwri      p.dma_cmd1_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM       // CQ completion to host
    phvwri      p.dma_cmd1_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(adminq_s2s_cmd_status)
    phvwri      p.dma_cmd1_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(adminq_s2s_cmd_data2)
    phvwri      p.dma_cmd1_dma_cmd_host_addr, 1                             // The source address is a Host address
    phvwri      p.dma_cmd1_dma_cmd_cache, 0                                 // AXI Uncached Access
    phvwr       p.dma_cmd1_dma_cmd_addr, k.adminq_to_s1_cq_ring_base
    phvwri      p.dma_cmd1_dma_cmd_eop, 1

    nop.e
    nop

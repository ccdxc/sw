#include "INGRESS_p.h"
#include "ingress.h"
#include "cpu-table.h"

struct phv_ p;
struct cpu_tx_write_pkt_k k;

%%
    .align
cpu_tx_write_pkt_start:
    CAPRI_CLEAR_TABLE0_VALID

dma_cmd_intrinsic:
    phvwri  p.p4_intr_global_tm_iport, 9
    phvwri  p.p4_intr_global_tm_oport, 11
    phvwri  p.p4_intr_global_tm_oq, 0

    addi    r5, r0, CAPRI_PHV_START_OFFSET(dma_cmd0_dma_cmd_type) / 16
    add     r4, r5, r0
    phvwr   p.p4_txdma_intr_dma_cmd_ptr, r4

    phvwri  p.dma_cmd0_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    phvwr   p.dma_cmd0_dma_cmd_phv_start_addr, CPU_PHV_INTRINSIC_START
    phvwr   p.dma_cmd0_dma_cmd_phv_end_addr, CPU_PHV_INTRINSIC_END

dma_cmd_data:
    phvwri  p.dma_cmd1_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwri  p.dma_cmd1_dma_pkt_eop, 1
    
    CAPRI_OPERAND_DEBUG(k.to_s3_page_addr)
    CAPRI_OPERAND_DEBUG(k.to_s3_len)
    phvwr  p.dma_cmd1_dma_cmd_addr, k.to_s3_page_addr
    phvwr  p.dma_cmd1_dma_cmd_size, k.to_s3_len
    phvwr  p.dma_cmd1_dma_cmd_eop, 1

cpu_tx_write_pkt_done:
    nop.e
    nop

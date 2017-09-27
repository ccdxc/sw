#include "INGRESS_p.h"
#include "ingress.h"
#include "cpu-table.h"

struct phv_ p;
struct cpu_tx_write_pkt_k k;

%%
    .align
cpu_tx_write_pkt_start:
    CAPRI_CLEAR_TABLE_VALID(0)
    CAPRI_OPERAND_DEBUG(k.to_s5_page_addr)
    CAPRI_OPERAND_DEBUG(k.to_s5_len)
 
dma_cmd_intrinsic:
    phvwri  p.p4_intr_global_tm_iport, 9
    phvwri  p.p4_intr_global_tm_oport, 11
    phvwri  p.p4_intr_global_tm_oq, 0
    phvwr   p.p4_intr_global_lif, k.to_s5_src_lif

    addi    r5, r0, CAPRI_PHV_START_OFFSET(dma_cmd0_dma_cmd_type) / 16
    add     r4, r5, r0
    phvwr   p.p4_txdma_intr_dma_cmd_ptr, r4

    phvwri  p.dma_cmd0_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    phvwr   p.dma_cmd0_dma_cmd_phv_start_addr, CPU_PHV_INTRINSIC_START
    phvwr   p.dma_cmd0_dma_cmd_phv_end_addr, CPU_PHV_INTRINSIC_END

cpu_tx_check_vlan_rewrite:
    sne     c1, k.common_phv_write_vlan_tag, r0
    bcf     [c1], dma_cmd_vlan_rewrite_header
    nop

dma_cmd_data:
    // Calculate offsets after removing CPU HDR 
    add     r4, k.to_s5_page_addr, CPU_TO_P4PLUS_HDR_SIZE
    sub     r5, k.to_s5_len, CPU_TO_P4PLUS_HDR_SIZE

    phvwri  p.dma_cmd1_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwri  p.dma_cmd1_dma_pkt_eop, 1
    
    phvwr   p.dma_cmd1_dma_cmd_addr, r4
    phvwr   p.dma_cmd1_dma_cmd_size, r5
    phvwr   p.dma_cmd1_dma_cmd_eop, 1
    b       cpu_tx_write_pkt_done
    nop

dma_cmd_vlan_rewrite_header:
    //  Start offset 
    add     r4, k.to_s5_page_addr, CPU_TO_P4PLUS_HDR_SIZE
    add     r5, r0, (P4PLUS_TO_P4_HDR_SIZE + L2HDR_DOT1Q_OFFSET)

    phvwri  p.dma_cmd1_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwr   p.dma_cmd1_dma_cmd_addr, r4
    phvwr   p.dma_cmd1_dma_cmd_size, r5

dma_cmd_vlan_header:
    phvwri  p.dma_cmd2_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    phvwr   p.dma_cmd2_dma_cmd_phv_start_addr, CPU_PHV_VLAN_TAG_START
    phvwr   p.dma_cmd2_dma_cmd_phv_end_addr, CPU_PHV_VLAN_TAG_END

dma_cmd_trailer:
    //  trailer start =header start + header size
    add     r4, r4, r5
    sub     r5, k.to_s5_len, r5
   
    // Remove any existing vlan tag from the packet
    sne     c1, k.to_s5_vlan_tag_exists, r0
    add.c1  r4, r4, VLAN_TAG_HDR_SIZE
    sub.c1  r5, r5, VLAN_TAG_HDR_SIZE

    phvwri  p.dma_cmd3_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwr   p.dma_cmd3_dma_cmd_addr, r4
    phvwr   p.dma_cmd3_dma_cmd_size, r5
    phvwri  p.dma_cmd3_dma_pkt_eop, 1
    phvwr   p.dma_cmd3_dma_cmd_eop, 1

cpu_tx_write_pkt_done:
    nop.e
    nop

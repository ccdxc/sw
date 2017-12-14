
#include "INGRESS_p.h"
#include "ingress.h"
#include "defines.h"
#include "../../p4/nw/include/defines.h"

struct phv_ p;
struct tx_table_s1_t0_k k;
struct tx_table_s1_t0_eth_tx_packet_d d;

%%

.align
eth_tx_packet:

    // Indicate VLAN insertion offload
    seq         c1, d.vlan_insert, 1
    addi.c1     r1, r0, P4PLUS_TO_P4_FLAGS_INSERT_VLAN_TAG
    phvwr.c1    p.eth_tx_app_hdr_vlan_tag, d.{vlan_tag}.hx

    // Set offload flags (Assume in R1)
    phvwr.c1    p.eth_tx_app_hdr_flags, r1

    // Set intrinsics
    phvwri      p.p4_intr_global_tm_iport, TM_PORT_DMA
    phvwri      p.p4_intr_global_tm_oport, TM_PORT_INGRESS
    phvwri      p.p4_intr_global_tm_oq, 0

    // Setup DMA CMD PTR
    phvwri      p.p4_txdma_intr_dma_cmd_ptr, ETH_DMA_CMD_PTR

    // End of pipeline - Make sure no more tables will be launched
    phvwri      p.{app_header_table0_valid...app_header_table3_valid}, 0

    // DMA cap_phv_intr_global_t (18B)
    phvwri      p.dma_cmd0_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    phvwri      p.dma_cmd0_dma_cmd_cmdsize, 2
    phvwri      p.dma_cmd0_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(p4_intr_global_tm_iport)
    phvwri      p.dma_cmd0_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(p4_intr_global_tm_instance_type)
    // DMA TxDMA intrinsic
    phvwri      p.dma_cmd0_dma_cmd_phv_start_addr1, CAPRI_PHV_START_OFFSET(p4_txdma_intr_qid)
    phvwri      p.dma_cmd0_dma_cmd_phv_end_addr1, CAPRI_PHV_END_OFFSET(p4_txdma_intr_txdma_rsv)
    // DMA p4plus_to_p4_header_t (14B)
    phvwri      p.dma_cmd0_dma_cmd_phv_start_addr2, CAPRI_PHV_START_OFFSET(eth_tx_app_hdr_p4plus_app_id)
    phvwri      p.dma_cmd0_dma_cmd_phv_end_addr2, CAPRI_PHV_END_OFFSET(eth_tx_app_hdr_vlan_tag)

    // DMA packet from Host Memory
    phvwri      p.dma_cmd1_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwri      p.dma_cmd1_dma_pkt_eop, 1
    phvwri      p.dma_cmd1_dma_cmd_host_addr, 1
    phvwr       p.dma_cmd1_dma_cmd_addr, d.{addr}.dx
    phvwr       p.dma_cmd1_dma_cmd_size, d.{len}.hx

    //DMA CQ descriptor to Host Memory
    phvwri      p.dma_cmd2_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri      p.dma_cmd2_dma_cmd_eop, 1
    phvwri      p.dma_cmd2_dma_cmd_host_addr, 1
    phvwr       p.dma_cmd2_dma_cmd_addr, k.eth_tx_to_s1_cq_desc_addr
    phvwri.e    p.dma_cmd2_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(eth_tx_cq_desc_completion_index)
    phvwri      p.dma_cmd2_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(eth_tx_cq_desc_color)

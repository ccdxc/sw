#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct rx_table_s4_t0_k k;
struct rx_table_s4_t0_ipsec_cb_tail_enqueue_input_desc_d d;
struct phv_ p;

%%
        .param IPSEC_CB_BASE
        .align

esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc:
    phvwr p.ipsec_int_header_in_desc, k.t0_s2s_in_desc_addr
    phvwri p.p4_rxdma_intr_dma_cmd_ptr, H2N_RXDMA_IPSEC_DMA_COMMANDS_OFFSET 
    phvwri p.app_header_table0_valid, 0
    phvwri p.app_header_table1_valid, 0
    phvwri p.app_header_table2_valid, 0
    phvwri p.app_header_table3_valid, 0

dma_cmd_to_write_ipsec_int_from_rxdma_to_txdma:
    phvwri p.dma_cmd_phv2mem_ipsec_int_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwr p.dma_cmd_phv2mem_ipsec_int_dma_cmd_addr, k.t0_s2s_in_desc_addr
    phvwri p.dma_cmd_phv2mem_ipsec_int_dma_cmd_phv_start_addr, IPSEC_INT_START_OFFSET
    phvwri p.dma_cmd_phv2mem_ipsec_int_dma_cmd_phv_end_addr, IPSEC_INT_END_OFFSET

dma_cmd_to_write_input_desc_aol:
    phvwri p.dma_cmd_in_desc_aol_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    add r1, r0, k.t0_s2s_in_desc_addr
    addi r1, r1, 64
    phvwr p.dma_cmd_in_desc_aol_dma_cmd_addr, r1
    phvwri p.dma_cmd_in_desc_aol_dma_cmd_phv_start_addr, IPSEC_IN_DESC_AOL_START
    phvwri p.dma_cmd_in_desc_aol_dma_cmd_phv_end_addr, IPSEC_IN_DESC_AOL_END
    
dma_cmd_to_write_output_desc_aol:
    phvwri p.dma_cmd_out_desc_aol_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    add r1, r0, k.ipsec_to_stage4_out_desc_addr 
    addi r1, r1, 64
    phvwr p.dma_cmd_out_desc_aol_dma_cmd_addr, r1 
    phvwri p.dma_cmd_out_desc_aol_dma_cmd_phv_start_addr, IPSEC_OUT_DESC_AOL_START
    phvwri p.dma_cmd_out_desc_aol_dma_cmd_phv_end_addr, IPSEC_OUT_DESC_AOL_END

dma_cmd_incr_pindex:
    add r2, r0, d.cb_pindex
    addi r2, r2, 1
    phvwr p.ipsec_global_ipsec_cb_pindex, r2
    add r3, r0, k.{ipsec_global_ipsec_cb_addr_sbit0_ebit31...ipsec_global_ipsec_cb_addr_sbit32_ebit33}
    addi r3, r3, IPSEC_CB_CB_PINDEX_OFFSET
    phvwri p.dma_cmd_incr_pindex_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwr p.dma_cmd_incr_pindex_dma_cmd_addr, r3
    phvwri p.dma_cmd_incr_pindex_dma_cmd_phv_start_addr, IPSEC_CB_PINDEX_START
    phvwri p.dma_cmd_incr_pindex_dma_cmd_phv_end_addr, IPSEC_CB_PINDEX_END  

dma_cmd_ring_doorbell:
    /* address will be in r4 */
    CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_INC, DB_SCHED_UPD_SET, 0, LIF_IPSEC_ESP)
        /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA(0, k.ipsec_global_ipsec_cb_index, 0, d.cb_pindex)                                                                                                                                      
        phvwr           p.doorbell_cmd_dma_cmd_addr, r4
        phvwr           p.db_data_index, d.cb_pindex 
        phvwr           p.db_data_qid, d.ipsec_cb_index

        phvwri          p.doorbell_cmd_dma_cmd_phv_start_addr,IPSEC_PHV_RXDMA_DB_DATA_START
        phvwri          p.doorbell_cmd_dma_cmd_phv_end_addr, IPSEC_PHV_RXDMA_DB_DATA_END
        phvwri          p.doorbell_cmd_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM

        phvwri          p.doorbell_cmd_dma_cmd_eop, 1
        phvwri          p.doorbell_cmd_dma_cmd_wr_fence, 1
        nop.e
        nop

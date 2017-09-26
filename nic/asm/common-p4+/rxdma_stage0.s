/*****************************************************************************
 *  rxdma_stage0.s: This assembly program provides the common entry point to 
 *                   jump to various RXDMA P4+ programs. 
 *                   1. Include the start labels used in your stage0 programs 
 *                      in both the parameter and in the jump instrunction. 
 *                   2. Use <this_program_name, stage0_label> when invoking 
 *                      Capri MPU loader API capri_program_label_to_offset() 
 *                      to derive the relative offset to program the queue 
 *                      context structure.
 *                   3. Use <this_program_name> when invoking Capri MPU
 *                      loader API capri_program_to_base_addr() to derive
 *                      the base address for the stage 0 P4+ programs in the
 *                      table config engine.
 *****************************************************************************/


%%
    .param      tcp_rx_read_shared_stage0_start
    .param      resp_rx_rqcb_process
    .param      cpu_rx_read_shared_stage0_start
    .param      eth_rx_fetch_desc
    .param      esp_ipv4_tunnel_h2n_ipsec_encap_rxdma_initial_table
    .param      esp_ipv4_tunnel_n2h_rxdma_initial_table


//Keep offset 0 for none to avoid invoking unrelated program when
//qstate's pc_offset is not initialized
.align
dummy:
    nop.e
    nop

//Do not change the order of this entry. 
//Keep it the first one after dummy
//This has to align with the txdma_stage0.s program
.align
rdma_resp_rx_stage0:
    j resp_rx_rqcb_process
    nop

//Do not change the order of this entry. 
//Keep it the second one after dummy
//This has to align with the txdma_stage0.s program
.align
rdma_req_rx_stage0:
    nop.e
    nop

.align
eth_rx_stage0:
    j eth_rx_fetch_desc
    nop

.align
tcp_rx_stage0:
    j tcp_rx_read_shared_stage0_start
    nop

.align
ipsec_rx_stage0:
    j esp_ipv4_tunnel_h2n_ipsec_encap_rxdma_initial_table 
    nop

.align
cpu_rx_stage0:
    j cpu_rx_read_shared_stage0_start
    nop

.align
ipsec_rx_n2h_stage0:
    j esp_ipv4_tunnel_n2h_rxdma_initial_table
    nop

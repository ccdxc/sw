/*****************************************************************************
 *  txdma_stage0.s: This assembly program provides the common entry point to 
 *                   jump to various TXDMA P4+ programs. 
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

#include "INGRESS_p.h"

struct phv_ p;

%%

    .param      req_tx_sqcb_process
    .param      storage_tx_q_state_pop_start
    .param      storage_seq_q_state_pop_start
    .param      storage_tx_pri_q_state_pop_start
    .param      tcp_tx_read_rx2tx_shared_process
    .param      tls_stage0
    .param      eth_tx_fetch_desc
    .param      eth_tx_arm_rx
    .param      esp_ipv4_tunnel_h2n_txdma_stage0
    .param      cpu_tx_stage0_start
#ifndef GFT
    .param      ipfix_start
#endif
    .param      resp_tx_rqcb_process
    .param      esp_ipv4_tunnel_n2h_txdma_stage0
    .param      adminq_fetch_desc
    .param      notify_fetch_desc
    .param      edma_fetch_desc
    .param      nicmgr_fetch_desc
    .param      nicmgr_drop
    .param      rawc_tx_start
    .param      gc_tx_rnmdr_s0_start
    .param      gc_tx_tnmdr_s0_start
    .param      storage_tx_pvm_roce_sq_cb_pop_start
    .param      storage_tx_roce_cq_cb_pop_start
    .param      proxyr_s0_tx_start
    .param      proxyc_s0_tx_start
    .param      virtio_tx_read_qstate_start
    .param      storage_nvme_check_sq_state_start
    .param      storage_nvme_pop_r2n_sq_start
    .param      storage_nvme_pop_arm_q_start
    .param      smbdc_req_tx_sqcb_process
    .param      rdma_cq_tx_cqcb_process
    .param      rdma_aq_tx_aqcb_process
    .param      tcp_ooq_load_qstate
#ifndef GFT
    .param      nvme_req_tx_sqcb_process
    .param      nvme_sessxts_tx_cb_process
    .param      nvme_sessdgst_tx_cb_process
    .param      nvme_sessxts_rx_cb_process
    .param      nvme_sessdgst_rx_cb_process
#endif
    .param      tcp_qtype1_process
    
//Keep offset 0 for none to avoid invoking unrelated program when
//qstate's pc_offset is not initialized
.align
tx_dummy:
   phvwr.e     p.p4_intr_global_drop, 1
   phvwri      p.{app_header_table0_valid...app_header_table3_valid}, 0

//Do not change the order of this entry. 
//Keep it the first one after dummy
//This has to align with the rxdma_stage0.s program
.align
rdma_resp_tx_stage0:
    j resp_tx_rqcb_process
    nop

//Do not change the order of this entry. 
//Keep it the second one after dummy
//This has to align with the rxdma_stage0.s program
.align
rdma_req_tx_stage0:
    j req_tx_sqcb_process
    nop

//Do not change the order of this entry
//This has to align with the rxdma_stage0.s program
.align
eth_tx_arm_rx_stage0:
    j eth_tx_arm_rx
    nop

//Do not change the order of this entry
//This has to align with the rxdma_stage0.s program
.align
eth_tx_stage0:
    j eth_tx_fetch_desc
    nop

//Do not change the order of this entry
//This has to align with the rxdma_stage0.s program
.align
rdma_cq_tx_stage0:
    j rdma_cq_tx_cqcb_process
    nop

//Do not change the order of this entry
//This has to align with the rxdma_stage0.s program
.align
rdma_aq_tx_stage0:
    j rdma_aq_tx_aqcb_process
    nop

.align
tcp_tx_stage0:
    j   tcp_tx_read_rx2tx_shared_process
    nop

.align
ipsec_tx_stage0:
    j esp_ipv4_tunnel_h2n_txdma_stage0
    nop

.align
tls_tx_stage0:
    j   tls_stage0
    nop

.align 
ipsec_tx_n2h_stage0:
    j esp_ipv4_tunnel_n2h_txdma_stage0
    nop

.align
storage_tx_stage0:
   j    storage_tx_q_state_pop_start
   nop

.align
storage_tx_pri_stage0:
   j    storage_tx_pri_q_state_pop_start
   nop

.align
storage_tx_rsq_stage0:
   j    storage_tx_pvm_roce_sq_cb_pop_start
   nop

.align
storage_tx_rcq_stage0:
   j    storage_tx_roce_cq_cb_pop_start
   nop

.align
cpu_tx_stage0:
    j   cpu_tx_stage0_start
    nop

#ifndef GFT
.align
ipfix_tx_stage0:
    j   ipfix_start
    nop
#endif

.align
adminq_stage0:
    j   adminq_fetch_desc
    nop

.align
notify_stage0:
    j   notify_fetch_desc
    nop

.align
edma_stage0:
    j   edma_fetch_desc
    nop

.align
nicmgr_req_stage0:
    j   nicmgr_drop
    nop

.align
nicmgr_resp_stage0:
    j   nicmgr_fetch_desc
    nop

.align
rawc_tx_stage0:
    j   rawc_tx_start
    nop

.align
gc_tx_rnmdr_stage0:
    j   gc_tx_rnmdr_s0_start
    nop

.align
gc_tx_tnmdr_stage0:
    j   gc_tx_tnmdr_s0_start
    nop
    
.align
proxyr_tx_stage0:
    j   proxyr_s0_tx_start
    nop
    
.align
proxyc_tx_stage0:
    j   proxyc_s0_tx_start
    nop

.align
virtio_tx_stage0:
    j virtio_tx_read_qstate_start
    nop

.align
storage_nvme_sq_stage0:
    j storage_nvme_check_sq_state_start
    nop

.align
storage_nvme_r2n_sq_stage0:
    j storage_nvme_pop_r2n_sq_start
    nop

.align
storage_nvme_armq_stage0:
    j storage_nvme_pop_arm_q_start
    nop

.align
smbdc_req_tx_stage0:
    j smbdc_req_tx_sqcb_process
    nop

.align
storage_seq_stage0:
   j    storage_seq_q_state_pop_start
   nop

.align
tcp_ooq_rx2tx_stage0:
    j tcp_qtype1_process
    nop

#ifndef GFT
.align
nvme_sq_stage0:
    j nvme_req_tx_sqcb_process
    nop

.align
nvme_tx_sessxts_stage0:
    j nvme_sessxts_tx_cb_process
    nop

.align
nvme_tx_sessdgst_stage0:
    j nvme_sessdgst_tx_cb_process
    nop

.align
nvme_rq_stage0:
    #j nvme_req_rx_process
    nop.e
    nop

.align
nvme_rx_sessxts_stage0:
    #j nvme_sessxts_rx_cb_process
    nop.e
    nop

.align
nvme_rx_sessdgst_stage0:
    #j nvme_sessdgst_rx_cb_process
    nop.e
    nop
#endif

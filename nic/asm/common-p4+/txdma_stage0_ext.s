/*****************************************************************************
 *  txdma_stage0_ext.s: This assembly program provides the common entry point to 
 *                   jump to various TXDMA P4+ programs. This is an extension
 *                   program for performance reasons.
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

    .param      req_tx_sqcb_process_ext
    .param      resp_tx_rqcb_process_ext
    .param      tcp_tx_read_rx2tx_shared_process_ext
    .param      smbdc_req_tx_sqcb_process_ext

//Keep offset 0 for none to avoid invoking unrelated program when
//qstate's pc_offset is not initialized
.align
dummy:
   nop.e
   nop

//Do not change the order of this entry. 
//Keep it the first one after dummy
//This has to align with the rxdma_stage0.s program
.align
rdma_resp_tx_stage0:
    j resp_tx_rqcb_process_ext
    nop

//Do not change the order of this entry. 
//Keep it the second one after dummy
//This has to align with the rxdma_stage0.s program
.align
rdma_req_tx_stage0:
    j req_tx_sqcb_process_ext
    nop

//Do not change the order of this entry
//This has to align with the rxdma_stage0.s program
.align
eth_rx_stage0_dummy:
    nop.e
    nop

//Do not change the order of this entry
//This has to align with the rxdma_stage0.s program
.align
eth_tx_stage0:
    nop.e
    nop

.align
tcp_tx_stage0:
    j tcp_tx_read_rx2tx_shared_process_ext
    nop

.align
ipsec_tx_stage0:
    nop.e
    nop

.align
tls_tx_stage0:
    nop.e
    nop

.align 
ipsec_tx_n2h_stage0:
    nop.e
    nop

.align
storage_tx_stage0:
   nop.e
   nop

.align
storage_tx_pri_stage0:
   nop.e
   nop

.align
storage_tx_rsq_stage0:
   nop.e
   nop

.align
storage_tx_rcq_stage0:
   nop.e
   nop

.align
cpu_tx_stage0:
    nop.e
    nop

#ifndef GFT
.align
ipfix_tx_stage0:
    nop.e
    nop
#endif

.align
adminq_stage0:
    nop.e
    nop

.align
rawc_tx_stage0:
    nop.e
    nop

.align
gc_tx_rnmdr_stage0:
    nop.e
    nop

.align
gc_tx_tnmdr_stage0:
    nop.e
    nop
    
.align
proxyr_tx_stage0:
    nop.e
    nop
    
.align
proxyc_tx_stage0:
    nop.e
    nop

.align
virtio_tx_stage0:
    nop.e
    nop

.align
storage_nvme_sq_stage0:
    nop.e
    nop

.align
storage_nvme_r2n_sq_stage0:
    nop.e
    nop

.align
storage_nvme_armq_stage0:
    nop.e
    nop

.align
smbdc_req_tx_stage0:
    j smbdc_req_tx_sqcb_process_ext
    nop

.align
storage_seq_stage0:
   nop.e
   nop


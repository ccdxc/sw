/*****************************************************************************
 * storage_seq.p4: Sequencer functions used by storage_tx P4+ program
 *****************************************************************************/


/*****************************************************************************
 *  seq_pdma_entry_handler: Handle the PDMA entry in sequencer. Issue the DMA
 *                          command to copy data and ring the next doorbell 
 *                          with a fence bit to guarantee ordering.
 *****************************************************************************/

action seq_pdma_entry_handler(next_db_addr, next_db_data, src_addr, dst_addr,
                              data_size, src_lif_override, src_lif,
                              dst_lif_override, dst_lif, intr_addr, intr_data,
                              next_db_en, intr_en) {

  // For D vector generation (type inference). No need to translate this to ASM.
  modify_field(seq_pdma_entry_scratch.next_db_addr, next_db_addr);
  modify_field(seq_pdma_entry_scratch.next_db_data, next_db_data);
  modify_field(seq_pdma_entry_scratch.src_addr, src_addr);
  modify_field(seq_pdma_entry_scratch.dst_addr, dst_addr);
  modify_field(seq_pdma_entry_scratch.data_size, data_size);
  modify_field(seq_pdma_entry_scratch.src_lif_override, src_lif_override);
  modify_field(seq_pdma_entry_scratch.src_lif, src_lif);
  modify_field(seq_pdma_entry_scratch.dst_lif_override, dst_lif_override);
  modify_field(seq_pdma_entry_scratch.dst_lif, dst_lif);
  modify_field(seq_pdma_entry_scratch.intr_addr, intr_addr);
  modify_field(seq_pdma_entry_scratch.intr_data, intr_data);
  modify_field(seq_pdma_entry_scratch.next_db_en, next_db_en);
  modify_field(seq_pdma_entry_scratch.intr_en, intr_en);

  // Form the doorbell and setup the DMA command to pop the entry by writing 
  // w_ndx to c_ndx
  QUEUE_POP_DOORBELL_UPDATE

  // Setup the DMA command to move the data from source to destination address
  // In ASM, set the host, fence bits etc correctly
  DMA_COMMAND_MEM2MEM_FILL(dma_m2m_1, dma_m2m_2, seq_pdma_entry_scratch.src_addr, 0,
                           seq_pdma_entry_scratch.dst_addr, 0, seq_pdma_entry_scratch.data_size,
                           0, 0, 0)


  // Setup the doorbell to be rung if the doorbell enabled is set.
  // Fence with the previous mem2mem DMA for ordering.
  if (seq_pdma_entry_scratch.next_db_en == 1) {
    // Copy the doorbell addr and data
    modify_field(doorbell_addr_scratch.addr, seq_pdma_entry_scratch.next_db_addr);
    modify_field(qpush_doorbell_data.data, seq_pdma_entry_scratch.next_db_data);
    DMA_COMMAND_PHV2MEM_FILL(dma_p2m_3, 
                             0,
                             PHV_FIELD_OFFSET(seq_doorbell_data.data),
                             PHV_FIELD_OFFSET(seq_doorbell_data.data),
                             0, 0, 0, 0)
  }

  // Fire the interrupt if there is no doorbell to be rung and if the
  // interrupt enabled bit is set. Fence with the previous mem2mem DMA
  // for ordering.
  if ((seq_pdma_entry_scratch.next_db_en ==  0) and 
      (seq_pdma_entry_scratch.intr_en == 1)) {
    // Copy the doorbell addr and data
    modify_field(pci_intr_addr_scratch.addr, seq_pdma_entry_scratch.intr_addr);
    modify_field(pci_intr_data.data, seq_pdma_entry_scratch.intr_data);
    DMA_COMMAND_PHV2MEM_FILL(dma_p2m_3, 
                             0,
                             PHV_FIELD_OFFSET(pci_intr_data.data),
                             PHV_FIELD_OFFSET(pci_intr_data.data),
                             0, 0, 0, 0)
  }

  // Exit the pipeline here 
}

/*****************************************************************************
 *  seq_r2n_entry_handler: Handle the R2N WQE entry in sequencer. Form the DMA
 *                         command to copy the R2N WQE as part of the push 
 *                         operation in the next stage.
 *****************************************************************************/

action seq_r2n_entry_handler(r2n_wqe_addr, r2n_wqe_size, dst_lif, dst_qtype,
                             dst_qid, dst_qaddr, is_remote) {

  // For D vector generation (type inference). No need to translate this to ASM.
  SEQ_R2N_ENTRY_COPY(seq_r2n_entry_scratch)

  // Form the doorbell and setup the DMA command to pop the entry by writing 
  // w_ndx to c_ndx
  QUEUE_POP_DOORBELL_UPDATE

  // Overwrite the K+I vector for the push operation to derive the correct 
  // destination queue
  modify_field(storage_kivec0.dst_lif, dst_lif);
  modify_field(storage_kivec0.dst_qtype, dst_qtype);
  modify_field(storage_kivec0.dst_qid, dst_qid);
  modify_field(storage_kivec0.dst_qaddr, dst_qaddr);

  // Setup the DMA command to move the data from source to destination address
  // In ASM, set the host, fence bits etc correctly
  DMA_COMMAND_MEM2MEM_FILL(dma_m2m_1, dma_m2m_2, seq_r2n_entry_scratch.r2n_wqe_addr, 0,
                           0, 0, seq_r2n_entry_scratch.r2n_wqe_size, 0, 0, 0)

  if (seq_r2n_entry_scratch.is_remote == 1) {
    // Load the Serivce SQ context for the next stage to push the NVME command
    CAPRI_LOAD_TABLE_ADDR(common_te0_phv, storage_kivec0.dst_qaddr,
                          Q_STATE_SIZE, seq_pvm_roce_sq_cb_push_start)
  } else {
    // Load the Serivce SQ context for the next stage to push the NVME command
    CAPRI_LOAD_TABLE_ADDR(common_te0_phv, storage_kivec0.dst_qaddr,
                          Q_STATE_SIZE, seq_q_state_push_start)
  }
}


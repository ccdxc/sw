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


/*****************************************************************************
 *  seq_barco_entry_handler: Handle the Barco XTS entry in sequencer. Form the
 *                           DMA command to copy the Barco XTS descriptor as 
 *                           part of the push operation in the next stage.
 *****************************************************************************/

action seq_barco_entry_handler(xts_desc_addr, xts_desc_size, xts_pndx_size,
                               xts_pndx_addr, xts_ring_addr) {

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // For D vector generation (type inference). No need to translate this to ASM.
  modify_field(seq_barco_entry_scratch.xts_desc_addr, xts_desc_addr);
  modify_field(seq_barco_entry_scratch.xts_desc_size, xts_desc_size);
  modify_field(seq_barco_entry_scratch.xts_pndx_size, xts_pndx_size);
  modify_field(seq_barco_entry_scratch.xts_pndx_addr, xts_pndx_addr);
  modify_field(seq_barco_entry_scratch.xts_ring_addr, xts_ring_addr);

  // Update the K+I vector with the barco descriptor size to be used 
  // when calculating the offset for the push operation 
  modify_field(storage_kivec1.xts_desc_size, seq_barco_entry_scratch.xts_desc_size);
  modify_field(storage_kivec1.device_addr, seq_barco_entry_scratch.xts_ring_addr);

  // Form the doorbell and setup the DMA command to pop the entry by writing 
  // w_ndx to c_ndx
  QUEUE_POP_DOORBELL_UPDATE

  // Setup the DMA command to move the data from source to destination address
  // In ASM, set the host, fence bits etc correctly
  DMA_COMMAND_MEM2MEM_FILL(dma_m2m_1, dma_m2m_2, seq_barco_entry_scratch.xts_desc_addr, 0,
                           0, 0, seq_barco_entry_scratch.xts_desc_size, 0, 0, 0)

  // Setup the doorbell to be rung based on a fence with the previous mem2mem 
  // DMA. Form the doorbell DMA command in this stage as opposed the push 
  // stage (as is the norm) to avoid carrying the doorbell address in K+I
  // vector.
  modify_field(doorbell_addr_scratch.addr, seq_barco_entry_scratch.xts_pndx_addr);
  DMA_COMMAND_PHV2MEM_FILL(dma_p2m_3, 
                           0,
                           PHV_FIELD_OFFSET(qpush_doorbell_data.data),
                           PHV_FIELD_OFFSET(qpush_doorbell_data.data),
                           0, 0, 0, 0)

  // Load the Barco ring for the next stage to push the Barco XTS descriptor
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, 
                        seq_barco_entry_scratch.xts_pndx_addr,
                        seq_barco_entry_scratch.xts_pndx_size, 
                        seq_barco_ring_push_start)
}

/*****************************************************************************
 *  seq_barco_ring_push: Push to Barco XTS ring by issuing the mem2mem DMA 
 *                       commands and incrementing the p_ndx via ringing the 
 *                       doorbell. Assumes that data to be pushed has its 
 *                       source in DMA cmd 1 and destination in DMA cmd 2.
 *****************************************************************************/

@pragma little_endian p_ndx
action seq_barco_ring_push(p_ndx) {

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // For D vector generation (type inference). No need to translate this to ASM.
  modify_field(barco_xts_ring_scratch.p_ndx, p_ndx);

  // Copy the doorbell data to PHV
  modify_field(qpush_doorbell_data.data, barco_xts_ring_scratch.p_ndx);

  // Modify the DMA command 2 to fill the destination address based on p_ndx 
  // NOTE: This API in P4 land will not work, but in ASM we can selectively
  // overwrite the fields
  DMA_COMMAND_PHV2MEM_FILL(dma_m2m_2, 
                           storage_kivec1.device_addr +
                           (barco_xts_ring_scratch.p_ndx * 
                            storage_kivec1.xts_desc_size),
                           0, 0,
                           0, 0, 0, 0)


  // Doorbell has already been setup

  // Exit the pipeline here 
}


/*****************************************************************************
 *  seq_comp_desc_handler: Handle the compression descriptor entry in the 
 *                         sequencer. This involves:
 *                          1. processing status to see if operation succeeded
 *                          2. breaking up the compressed data into the 
 *                             destination SGL provided in the descriptor
 *                         In this stage, load the status entry for next stage
 *                         and save the other fields into I vector.
 *****************************************************************************/

//@pragma little_endian next_db_addr next_db_data status_addr data_addr sgl_addr intr_addr intr_data status_len data_len
action seq_comp_desc_handler(next_db_addr, next_db_data, status_addr, data_addr, 
                             sgl_addr, intr_addr, intr_data, status_len, data_len,
                             data_len_from_desc, status_dma_en, next_db_en, intr_en) {

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC4_USE(storage_kivec4_scratch, storage_kivec4)
  STORAGE_KIVEC5_USE(storage_kivec5_scratch, storage_kivec5)

  // For D vector generation (type inference). No need to translate this to ASM.
  modify_field(seq_comp_desc_scratch.next_db_addr, next_db_addr);
  modify_field(seq_comp_desc_scratch.next_db_data, next_db_data);
  modify_field(seq_comp_desc_scratch.status_addr, status_addr);
  modify_field(seq_comp_desc_scratch.data_addr, data_addr);
  modify_field(seq_comp_desc_scratch.sgl_addr, sgl_addr);
  modify_field(seq_comp_desc_scratch.intr_addr, intr_addr);
  modify_field(seq_comp_desc_scratch.intr_data, intr_data);
  modify_field(seq_comp_desc_scratch.status_len, status_len);
  modify_field(seq_comp_desc_scratch.data_len, data_len);
  modify_field(seq_comp_desc_scratch.data_len_from_desc, data_len_from_desc);
  modify_field(seq_comp_desc_scratch.status_dma_en, status_dma_en);
  modify_field(seq_comp_desc_scratch.next_db_en, next_db_en);
  modify_field(seq_comp_desc_scratch.intr_en, intr_en);

  // Store the various parts of the descriptor in the K+I vectors for later use
  modify_field(storage_kivec4.sgl_addr, seq_comp_desc_scratch.sgl_addr);
  modify_field(storage_kivec4.data_addr, seq_comp_desc_scratch.data_addr);
  modify_field(storage_kivec4.data_len, seq_comp_desc_scratch.data_len);
  modify_field(storage_kivec5.status_addr, seq_comp_desc_scratch.status_addr);
  modify_field(storage_kivec5.status_len, seq_comp_desc_scratch.status_len);
  modify_field(storage_kivec5.status_dma_en, seq_comp_desc_scratch.status_dma_en);
  modify_field(storage_kivec5.data_len_from_desc, seq_comp_desc_scratch.data_len_from_desc);

  // Setup the doorbell to be rung if the doorbell enabled is set.
  // Fence with the SGL mem2mem DMA for ordering.
  if (seq_comp_desc_scratch.next_db_en == 1) {
    // Copy the doorbell addr and data
    modify_field(doorbell_addr_scratch.addr, seq_comp_desc_scratch.next_db_addr);
    modify_field(seq_doorbell_data.data, seq_comp_desc_scratch.next_db_addr);
    DMA_COMMAND_PHV2MEM_FILL(dma_p2m_11, 
                             0,
                             PHV_FIELD_OFFSET(seq_doorbell_data.data),
                             PHV_FIELD_OFFSET(seq_doorbell_data.data),
                             0, 0, 0, 0)
  }

  // Fire the interrupt if there is no doorbell to be rung and if the
  // interrupt enabled bit is set. Fence with the SGL mem2mem DMA
  // for ordering.
  if ((seq_comp_desc_scratch.next_db_en ==  0) and 
      (seq_comp_desc_scratch.intr_en == 1)) {
    // Copy the doorbell addr and data
    modify_field(pci_intr_addr_scratch.addr, seq_comp_desc_scratch.intr_addr);
    modify_field(pci_intr_data.data, seq_comp_desc_scratch.intr_data);
    DMA_COMMAND_PHV2MEM_FILL(dma_p2m_11, 
                             0,
                             PHV_FIELD_OFFSET(pci_intr_data.data),
                             PHV_FIELD_OFFSET(pci_intr_data.data),
                             0, 0, 0, 0)
  }

  // Form the doorbell and setup the DMA command to pop the entry by writing 
  // w_ndx to c_ndx
  QUEUE_POP_DOORBELL_UPDATE

  // Load the address where compression status is stored for processing 
  // in the next stage
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, 
                        seq_comp_desc_scratch.status_addr,
                        STORAGE_DEFAULT_TBL_LOAD_SIZE, 
                        seq_comp_status_handler_start)
}

/*****************************************************************************
 *  seq_comp_status_handler: Store the compression status in K+I vector. Load
 *                           SGL address for next stage to do the PDMA.
 *****************************************************************************/

@pragma little_endian data_len rsvd3
action seq_comp_status_handler(rsvd2, err, rsvd1, data_len, rsvd3) {

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC4_USE(storage_kivec4_scratch, storage_kivec4)
  STORAGE_KIVEC5_USE(storage_kivec5_scratch, storage_kivec5)

  // For D vector generation (type inference). No need to translate this to ASM.
  modify_field(seq_comp_status_scratch.rsvd2, rsvd2);
  modify_field(seq_comp_status_scratch.err, err);
  modify_field(seq_comp_status_scratch.rsvd1, rsvd1);
  modify_field(seq_comp_status_scratch.data_len, data_len);
  modify_field(seq_comp_status_scratch.rsvd3, rsvd3);

  // Store the data length in the K+I vector for later use if the descriptor
  // has not provided this information
  if (storage_kivec5.data_len_from_desc == 0) {
    modify_field(storage_kivec4.data_len, seq_comp_status_scratch.data_len);
  }
  // Store the error status in the K+I vector for later use
  modify_field(storage_kivec5.status_err, seq_comp_status_scratch.err);

  // Load the address where compression destination SGL is stored for 
  // processing in the next stage
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, 
                        storage_kivec4.sgl_addr,
                        STORAGE_DEFAULT_TBL_LOAD_SIZE, 
                        seq_comp_sgl_handler_start)
}

/*****************************************************************************
 *  seq_comp_sgl_handler: Parse the destination SGL and DMA the status, 
 *                        data (if status was success) and set the interrupt.
 *****************************************************************************/

@pragma little_endian status_addr addr0 addr1 addr2 addr3 len0 len1 len2 len3 
action seq_comp_sgl_handler(status_addr, addr0, addr1, addr2, addr3, 
                            len0, len1, len2, len3) {

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC4_USE(storage_kivec4_scratch, storage_kivec4)
  STORAGE_KIVEC5_USE(storage_kivec5_scratch, storage_kivec5)

  // For D vector generation (type inference). No need to translate this to ASM.
  modify_field(seq_comp_sgl_scratch.status_addr, status_addr);
  modify_field(seq_comp_sgl_scratch.addr0, addr0);
  modify_field(seq_comp_sgl_scratch.addr1, addr1);
  modify_field(seq_comp_sgl_scratch.addr2, addr2);
  modify_field(seq_comp_sgl_scratch.addr3, addr3);
  modify_field(seq_comp_sgl_scratch.len0, len0);
  modify_field(seq_comp_sgl_scratch.len1, len1);
  modify_field(seq_comp_sgl_scratch.len2, len2);
  modify_field(seq_comp_sgl_scratch.len3, len3);

  // DMA of status (only if enabled)
  if (storage_kivec5.status_dma_en != 0) {
    DMA_COMMAND_MEM2MEM_FILL(dma_m2m_1, dma_m2m_2, 
                             storage_kivec5.status_addr, 0,
                             seq_comp_sgl_scratch.status_addr, 0,
                             storage_kivec5.status_len,
                             0, 0, 0)
  }

  // DMA the data only if compression was successful
  if (seq_comp_status_scratch.err != 0) {
    exit();
  }

  // DMA to SGL 0
  if (storage_kivec4.data_len <= seq_comp_sgl_scratch.len0) {
    DMA_COMMAND_MEM2MEM_FILL(dma_m2m_3, dma_m2m_4, 
                             storage_kivec4.data_addr, 0,
                             seq_comp_sgl_scratch.addr0, 0,
                             storage_kivec4.data_len,
                             0, 0, 0)
    exit();
  } else {
    DMA_COMMAND_MEM2MEM_FILL(dma_m2m_3, dma_m2m_4, 
                             storage_kivec4.data_addr, 0,
                             seq_comp_sgl_scratch.addr0, 0,
                             seq_comp_sgl_scratch.len0,
                             0, 0, 0)
    modify_field(storage_kivec4.data_len, 
                 (storage_kivec4.data_len - seq_comp_sgl_scratch.len0));
  }

  // DMA to SGL 1
  if (storage_kivec4.data_len <= seq_comp_sgl_scratch.len1) {
    DMA_COMMAND_MEM2MEM_FILL(dma_m2m_5, dma_m2m_6, 
                             storage_kivec4.data_addr, 0,
                             seq_comp_sgl_scratch.addr1, 0,
                             storage_kivec4.data_len,
                             0, 0, 0)
    exit();
  } else {
    DMA_COMMAND_MEM2MEM_FILL(dma_m2m_5, dma_m2m_6, 
                             storage_kivec4.data_addr, 0,
                             seq_comp_sgl_scratch.addr1, 0,
                             seq_comp_sgl_scratch.len1,
                             0, 0, 0)
    modify_field(storage_kivec4.data_len, 
                 (storage_kivec4.data_len - seq_comp_sgl_scratch.len1));
  }

  // DMA to SGL 2
  if (storage_kivec4.data_len <= seq_comp_sgl_scratch.len2) {
    DMA_COMMAND_MEM2MEM_FILL(dma_m2m_7, dma_m2m_8, 
                             storage_kivec4.data_addr, 0,
                             seq_comp_sgl_scratch.addr2, 0,
                             storage_kivec4.data_len,
                             0, 0, 0)
    exit();
  } else {
    DMA_COMMAND_MEM2MEM_FILL(dma_m2m_7, dma_m2m_8, 
                             storage_kivec4.data_addr, 0,
                             seq_comp_sgl_scratch.addr2, 0,
                             seq_comp_sgl_scratch.len2,
                             0, 0, 0)
    modify_field(storage_kivec4.data_len, 
                 (storage_kivec4.data_len - seq_comp_sgl_scratch.len2));
  }

  // DMA to SGL 3
  if (storage_kivec4.data_len <= seq_comp_sgl_scratch.len3) {
    DMA_COMMAND_MEM2MEM_FILL(dma_m2m_9, dma_m2m_10, 
                             storage_kivec4.data_addr, 0,
                             seq_comp_sgl_scratch.addr3, 0,
                             storage_kivec4.data_len,
                             0, 0, 0)
    exit();
  } else {
    DMA_COMMAND_MEM2MEM_FILL(dma_m2m_9, dma_m2m_10, 
                             storage_kivec4.data_addr, 0,
                             seq_comp_sgl_scratch.addr3, 0,
                             seq_comp_sgl_scratch.len3,
                             0, 0, 0)
    modify_field(storage_kivec4.data_len, 
                 (storage_kivec4.data_len - seq_comp_sgl_scratch.len3));
  }

  // Exit the pipeline here
}

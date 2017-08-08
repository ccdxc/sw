/************************************************************
 *   tls_rx_serq.p4
 * This programs TLS model for packets received from SERQ. 
 ************************************************************/

#include "common/tls-serq-to-brq-metadata.h"

#define tx_table_s0_t0 tlscb_tbl

#define tx_table_s0_t0_action tls_rx_serq_action

#include "../../../../../common-p4+/p4/common_txdma.p4"

/*
 * tls_rx_serq_action
 *    This action is triggered after reading a serq entry and reading the
 *    contents of the descriptor and reading the TLS state
 */
action tls_rx_serq_action(key, salt, iv, cipher_type, ver_major, ver_minor, key_addr, iv_addr, qhead, qtail,
       			          una_desc, una_desc_idx, una_data_offset, una_data_len, 
                          nxt_desc, nxt_desc_idx, nxt_data_offset, nxt_data_len,
                          next_tls_hdr_offset, cur_tls_data_len, ofid) {
                          
			   
  /* Populate the descriptor header info from table data into phv */
  modify_field(dummy.key, key);
  modify_field(dummy.salt, salt);
  modify_field(dummy.iv, iv);
  modify_field(dummy.cipher_type, cipher_type);
  /*
  modify_field(dummy.ver_major, ver_major);
  modify_field(dummy.ver_minor, ver_minor);

  modify_field(dummy.key_addr, key_addr);
  modify_field(dummy.iv_addr, iv_addr);
  modify_field(dummy.qhead, qhead);
  modify_field(dummy.qtail, qtail);

  modify_field(dummy.una_desc, una_desc);
  modify_field(dummy.una_desc_idx, una_desc_idx);
  modify_field(dummy.una_data_offset, una_data_offset);
  modify_field(dummy.una_data_len, una_data_len);

  modify_field(dummy.nxt_desc, nxt_desc);
  modify_field(dummy.nxt_desc_idx, nxt_desc_idx);
  modify_field(dummy.nxt_data_offset, nxt_data_offset);
  modify_field(dummy.nxt_data_len, nxt_data_len);

  
  modify_field(dummy.next_tls_hdr_offset, next_tls_hdr_offset);
  modify_field(dummy.cnur_tls_data_len, cur_tls_data_len);
  modify_field(dummy.ofid, ofid);
  if (dummy.pending_read_header == 1) {
    CAPRI_LOAD_TABLE_ADDR(intrinsic, dummy.pending_read_addr, 5, tls_read_header)
  }

  if (dummy.pending_alloc_rnmdr == 1) {
    CAPRI_LOAD_TABLE_IDX(intrinsic, RNMDR_TABLE_BASE, serq_to_brq.rnmdr_alloc_idx, 8, tls_alloc_rnmdr)
  }
  */
}





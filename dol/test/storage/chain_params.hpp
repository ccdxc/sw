#ifndef _CHAIN_PARAMS_HPP_
#define _CHAIN_PARAMS_HPP_

#include <stdint.h>

namespace tests {

// P4+ TxDMA max MEM2MEM transfer size is limited to 14 bits.
// Note: HW does *NOT* equate zero to 16K!
const static uint32_t  kMaxMem2MemSize         = ((1 << 14) - 1);

#define ARRAYSIZE(a)    (sizeof((a)) / sizeof((a)[0]))

/*
 * Chaining next doorbell specification
 */
typedef struct {
  uint64_t next_doorbell_addr;	// Next capri doorbell address
  uint64_t next_doorbell_data;	// Next capri doorbell data
} chain_next_db_spec_t;

/*
 * Chaining Barco push specification
 */
typedef struct {
  uint64_t barco_ring_addr;     // ring address
  uint64_t barco_pndx_addr;     // producer index address
  uint64_t barco_pndx_shadow_addr;// producer index shadow address
  uint64_t barco_desc_addr;     // descriptor to push
  uint8_t  barco_desc_size;     // log2(descriptor size)
  uint8_t  barco_pndx_size;     // log2)producer index size)
  uint8_t  barco_ring_size;     // log2(ring size)
  uint8_t  barco_num_descs;     // initial number of descriptors to xfer to Barco.
                                // In some error cases, it also serves as a count
                                // beyond which P4+ may locate the "alternate" set
                                // of descriptors.
} chain_barco_push_spec_t;

/*
 * Chaining sequencer specification
 */
typedef struct {
  uint32_t seq_q;	            // Sequencer queue
  uint32_t seq_status_q;	    // Status sequencer queue
  uint32_t seq_next_q;   	    // Next sequencer queue in chain
  uint32_t seq_next_status_q;	// Next status sequencer queue in chain
  uint64_t ret_doorbell_addr;	// Doorbell address that is formed for the Status sequencer
  uint64_t ret_doorbell_data;	// Doorbell data that is formed for the Status sequencer
  uint16_t ret_seq_status_index;
} chain_seq_spec_t;

/*
 * Compression chaining parameters
 */
typedef struct {
    chain_seq_spec_t                seq_spec;
    union {
        chain_next_db_spec_t        db_spec;
        chain_barco_push_spec_t     push_spec;
    };
    uint64_t status_addr0;
    uint64_t status_addr1;	    // Destination for the PDMA of status from status_addr0
    uint64_t comp_buf_addr;     // Compressed data buffer address (source of SGL PDMA)

    // post compression, options available are:
    // - PDMA compressed data to pdma_out_sgl_pa (sgl_pdma_en), or
    // - chain to next accelerator service which uses input/output AOL (aol_pad_en),
    // - chain to next accelerator service which uses SGL input (sgl_pad_en),
    //   where P4+ will modify addr/length fields in the AOL/SGL based on compression result
    uint64_t aol_src_vec_addr;
    uint64_t aol_dst_vec_addr;
    uint64_t sgl_vec_addr;	    // SGL vector for multi-block hash
    uint64_t pad_buf_addr;	    // pad buffer address
    uint64_t intr_addr;		    // MSI-X Interrupt address
    uint32_t intr_data;		    // MSI-X Interrupt data
    uint16_t status_len;		// Length of the status header
    uint16_t data_len;		    // Remaining data length of compression buffer
    uint8_t  status_offset0;    // Offset to add to status_addr0 before PDMA
    uint8_t  pad_boundary_shift;// log2(pad block length)
    uint16_t data_len_from_desc   :1,	// use desc data_len rather than output_data_len
             status_dma_en        :1,	// enable DMA of status to status_hbm_pa
    // NOTE: intr_en and next_doorbell_en can be enabled together.
    // When comp/decomp succeeds, Order of evaluation: 1. next_doorbell_en 2. intr_en.
    // When comp/decomp fails and stop_chain_on_error is set, intr_en will be honored
             next_doorbell_en     :1,	// enable chain doorbell
             intr_en              :1,	// enable intr_data write to intr_pa
             next_db_action_barco_push:1,	// next_db action is actually a Barco push
             stop_chain_on_error  :1, // stop chaining on error
             copy_src_dst_on_error:1,
    // NOTE: sgl_xfer_en and aol_len_pad_en are mutually exclusive.
    // Order of evaluation: 1. aol_len_pad_en 2. sgl_xfer_en
             aol_pad_en           :1, // enable AOL length padding
             sgl_pad_en           :1, // enable SGL length padding (e.g., for multi-block hash)
             sgl_pdma_en          :1, // enable data transfer from comp_buf_addr to aol_dst_vec_addr
             sgl_pdma_pad_only    :1, // enable pad-only fill mode, i.e., Comp engine writes
                                      // compressed output according to SGL, P4+ will fill
                                      // the last block with the right amount of pad data.
                                      // This mode requires sgl_pad_hash_en as P4+ will glean
                                      // the buffers info from the supplied sgl_vec_pa.
             desc_vec_push_en     : 1;// barco_desc_addr points to a vector of descriptors to be pushed
} chain_params_comp_t;

/*
 * XTS chaining parameters
 */
typedef struct {
    chain_seq_spec_t                seq_spec;
    union {
        chain_next_db_spec_t        db_spec;
        chain_barco_push_spec_t     push_spec;
    };
    uint64_t status_addr0;
    uint64_t status_addr1;	    // Destination for the PDMA of status from status_addr0
    uint64_t decr_buf_addr;	    // decrypted data buffer address (source of SGL PDMA)

    uint64_t comp_sgl_src_addr; // (de)compression source SGL address
    uint64_t sgl_pdma_dst_addr; // points to chain_sgl_pdma_t when sgl_pdma_en is set
    uint64_t intr_addr;		    // MSI-X Interrupt address
    uint32_t intr_data;		    // MSI-X Interrupt data
    uint16_t status_len;		// Length of the status header
    uint16_t data_len;		    // valid PDMA data length if sgl_pdma_len_from_desc is set
    uint8_t  status_offset0;    // Offset to add to status_addr0 before PDMA
    uint8_t  blk_boundary_shift;// log2(block size)
    uint16_t status_dma_en        :1,	// enable DMA of status to status_hbm_pa
    // NOTE: intr_en and next_doorbell_en can be enabled together.
    // When XTS succeeds, Order of evaluation: 1. next_doorbell_en 2. intr_en.
    // When XTS fails and stop_chain_on_error is set, intr_en will be honored
             next_doorbell_en     :1,	// enable chain doorbell
             intr_en              :1,	// enable intr_data write to intr_pa
             next_db_action_barco_push:1,	// next_db action is actually a Barco push
             stop_chain_on_error  :1, // stop chaining on error
             comp_len_update_en   :1, // enable compression length update
             comp_sgl_src_en      :1, // source of (de)compression is in SGL format
             comp_sgl_src_vec_en  :1, // comp_sgl_src_addr points to a vector
             sgl_pdma_en          :1, // enable data transfer from decr_buf_addr to sgl_pdma_dst_addr
             sgl_pdma_len_from_desc:1,// PDMA length is data_len above; otherwise use length from comp hdr
             desc_vec_push_en     :1; // barco_desc_addr points to a vector
} chain_params_xts_t;

/*
 * SGL PDMA pamameters
 */
typedef struct {
    uint64_t addr;		    // Destination Address in the SGL for chaining PDMA
    uint32_t len;		    // Length of the SGL element for chaining PDMA
} __attribute__((packed)) chain_sgl_pdma_tuple_t;

typedef struct {
    chain_sgl_pdma_tuple_t  tuple[4];
    uint64_t                pad[2];
} __attribute__((packed)) chain_sgl_pdma_t;


}  // namespace tests

#endif   // _CHAIN_PARAMS_HPP_

/*****************************************************************************
 *  seq_comp_status_handler: Store the compression status in K+I vector. Load
 *                           SGL address for next stage to do the PDMA.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s2_tbl_k k;
struct s2_tbl_seq_comp_status_handler_d d;
struct phv_ p;

/*
 * Registers usage
 * CAUTION: r1 is also implicitly used by LOAD_TABLE1_FOR_ADDR_PC_IMM()
 */
#define r_total_len                 r1  // above length plus padding
#define r_comp_data_len             r2  // compression output data length
#define r_pad_len                   r3  // padding length
#define r_pad_boundary              r4  // user specified padding boundary
#define r_num_blks                  r5  // number of hash blocks
#define r_last_blk_len              r6  // length of last block
#define r_last_sgl_p                r7  // pointer to last SGL

/*
 * Registers reuse, pre SGL updates
 */
#define r_comp_desc_p               r7  // pointer to chain descrriptor

/*
 * Registers reuse, post padding calculations
 */
#define r_status                    r_last_sgl_p     // comp status, briefly used at beginning
#define r_sgl_len_total             r_num_blks       // length total of all SGL tuple buffers
#define r_sgl_tuple_no              r_total_len      // SGL tuple number
#define r_sgl_field_p               r_comp_data_len  // pointer to an SGL field
#define r_pad_buf_addr              r_sgl_len_total  // pad buffer address

/*
 * Registers reuse, post padding application
 */
#define r_rl_len                    r_last_blk_len   // rate limit length for SEQ_RATE_LIMIT_...()
#define r_src_len                   r_last_sgl_p

/*
 * Registers reuse, on compression error
 */
#define r_alt_desc_addr             r_pad_boundary   // alternate descriptor address
#define r_src_qaddr                 r_last_sgl_p     // qstate address
 
/*
 * Local vars (due to registers shortage)
 */
#define l_data_len                  d.integ_data1
 
%%
    .param storage_seq_barco_ring_pndx_read
    .param storage_seq_comp_aol_pad_handler
    .param storage_seq_comp_sgl_pdma_xfer
    .param storage_seq_comp_sgl_pad_only_xfer
    .param storage_seq_comp_db_intr_override

storage_seq_comp_status_handler:
    
     CLEAR_TABLE0
     
    // c6 indicates whether padding is enabled
    seq         c6, SEQ_KIVEC5_PADDING_EN, 1
    add         r_pad_len, r0, r0
    
    // bit 15: valid bit, bits 14-12: error bits
    add         r_status, d.status, r0
    smeqh       c4, r_status, 0xf000, 0x8000
    bcf         [!c4], comp_error
    phvwr	p.comp_hdr_cksum, d.integ_data0        // delay slot
   
    // Note: output_data_len contains compressed data length plus header length.
    
    seq	        c3, SEQ_KIVEC5_DATA_LEN_FROM_DESC, 1
    cmov        r_comp_data_len, c3, SEQ_KIVEC5_DATA_LEN, d.output_data_len
    seq	        c3, r_comp_data_len, r0
    add.c3      r_comp_data_len, 65536, r0
    phvwr	p.seq_kivec5_data_len, r_comp_data_len
    tblwr.l	l_data_len, r_comp_data_len
    seq         c4, SEQ_KIVEC5_STATUS_LEN_NO_HDR, 0
    sub.c4      r_total_len, r_comp_data_len, SIZE_IN_BYTES(sizeof(struct seq_comp_hdr_t))
    phvwr	p.comp_hdr_data_len, r_total_len.hx
   
    // Preliminary padding calculations:
    // r_num_blks = (r_comp_data_len + r_pad_boundary - 1) / r_pad_boundary)
    // r_total_len = r_num_blks * r_pad_boundary
    // r_pad_len = r_total_len - r_comp_data_len
   
    sll         r_pad_boundary, 1, SEQ_KIVEC4_PAD_BOUNDARY_SHIFT
    add         r_num_blks, r_comp_data_len, r_pad_boundary
    sub         r_num_blks, r_num_blks, 1
    srl         r_num_blks, r_num_blks, SEQ_KIVEC4_PAD_BOUNDARY_SHIFT
    sll         r_total_len, r_num_blks, SEQ_KIVEC4_PAD_BOUNDARY_SHIFT
    sub         r_pad_len, r_total_len, r_comp_data_len
    add         r_last_blk_len, r_comp_data_len, r0
    mincr       r_last_blk_len, SEQ_KIVEC4_PAD_BOUNDARY_SHIFT, r0

    // Readjust r_total_len and r_pad_len if padding_en=0    
    add.!c6     r_total_len, r_comp_data_len, r0
    add.!c6     r_pad_len, r0, r0
    
    // If requested, do in-stage datain_len update in the descriptor.
    // This memory update must complete before storage_seq_barco_chain_action
    // transfers the decriptor(s) to Barco.
if4:
    bbeq        SEQ_KIVEC5_DESC_DLEN_UPDATE_EN, 0, endif4
    add         r_comp_desc_p, SEQ_KIVEC4_BARCO_DESC_ADDR, \
                SIZE_IN_BYTES(offsetof(struct comp_desc_le_t, datain_len)) // delay slot
    memwr.hx    r_comp_desc_p, r_total_len
    wrfence
endif4:    

    // In the per-block hash or encryption case, we now indicate to
    // storage_seq_barco_chain_action the correct number of descriptors.
    seq         c3, SEQ_KIVEC5_DESC_VEC_PUSH_EN, 1
    phvwr.c3    p.seq_kivec4_barco_num_descs, r_num_blks
    
    // Note that both SGL update and AOL update may be enabled, for the
    // following use case: compress-pad-encrypt where the compressed output
    // data (padded) is saved while at the same time passed to Barco
    // for encryption (compression uses SGL format while encryption uses AOL).
                     
    bbeq        SEQ_KIVEC5_AOL_UPDATE_EN, 0, possible_sgl_update
    phvwrpair   p.seq_kivec3_pad_len, r_pad_len, \
                p.seq_kivec3_last_blk_len, r_last_blk_len       // delay slot
    
    // AOL padding enabled: handle in the next stage due to low availability
    // of k-vec space which forces usage of a stage_2_stage (seq_kivec6).
    LOAD_TABLE_NO_LKUP_PC_IMM(3, storage_seq_comp_aol_pad_handler) 

possible_sgl_update:
    bbeq        SEQ_KIVEC5_SGL_UPDATE_EN, 0, possible_sgl_pdma_xfer
    phvwr       p.seq_kivec3_num_blks, r_num_blks       // delay slot

    // SGL update enabled:
    // Given a vector of SGLs, each prefilled with exactly one block addr and len,
    // i.e., addr0/len0 specifies one block of data, find the last applicable SGL
    // and apply padding.
    SEQ_METRICS_SET(sgl_update_reqs)
   
    // Note: each SGL tuple is expected to point to a block of size r_pad_boundary
    //
    // Calculate
    //   r_sgl_len_total = num_blks_per_sgl << pad_boundary_shift
    //   r_last_sgl_p = &sgl_vec_addr[r_comp_data_len / r_sgl_len_total]
    //   r_sgl_tuple_no = (r_comp_data_len % r_sgl_len_total) >> pad_boundary_shift
    //
    // Note: r_pad_boundary is a power of 2 but num_blks_per_sgl may not be 
    // (it either equals 1 or 3)

    seq         c3, SEQ_KIVEC5_SGL_SPARSE_FORMAT_EN, 1
    sll.c3      r_sgl_len_total, 1, SEQ_KIVEC4_PAD_BOUNDARY_SHIFT 
    sll.!c3     r_sgl_len_total, BARCO_SGL_NUM_TUPLES_MAX, SEQ_KIVEC4_PAD_BOUNDARY_SHIFT 
    div         r_last_sgl_p, r_comp_data_len, r_sgl_len_total
    add         r_last_sgl_p, SEQ_KIVEC2_SGL_VEC_ADDR, \
                r_last_sgl_p, BARCO_SGL_DESC_SIZE_SHIFT
    mod         r_sgl_tuple_no, r_comp_data_len, r_sgl_len_total
    srl         r_sgl_tuple_no, r_sgl_tuple_no, SEQ_KIVEC4_PAD_BOUNDARY_SHIFT
    phvwr       p.seq_kivec3_sgl_tuple_no, r_sgl_tuple_no

    // Note: by this time we know that, since r_comp_data_len is guaranteed
    // to be non-zero, a zero r_last_blk_len not only means padding is not
    // to be applied, we need to "truncate" the SGL vector at the right spot.
if6:
    bne         r_last_blk_len, r0, else6
    seq         c3, r_sgl_tuple_no, r0                          // delay slot
    
switch0:    
  .brbegin
    br          r_sgl_tuple_no[1:0]
    sub.c3      r_last_sgl_p, r_last_sgl_p, BARCO_SGL_DESC_SIZE // delay slot

  .brcase BARCO_SGL_TUPLE0

    // last_sgl_p->link = 0
    add         r_sgl_field_p, r_last_sgl_p, \
                SIZE_IN_BYTES(offsetof(struct barco_sgl_le_t, link))
    DMA_PHV2MEM_SETUP(barco_sgl_tuple2_len_update_link,
                      barco_sgl_tuple2_len_update_link,
                      r_sgl_field_p, dma_p2m_2)
    b           endif6
    nop

  .brcase BARCO_SGL_TUPLE1

    // zero out the rest of last_sgl_p starting from tuple1
    add         r_sgl_field_p, r_last_sgl_p, \
                SIZE_IN_BYTES(offsetof(struct barco_sgl_le_t, addr1))
    DMA_PHV2MEM_SETUP(barco_sgl_tuple0_len_update_null_addr1,
                      barco_sgl_tuple0_len_update_link,
                      r_sgl_field_p, dma_p2m_2)
    b           endif6
    nop
    
  .brcase BARCO_SGL_TUPLE2

    // zero out the rest of last_sgl_p starting from tuple2
    add         r_sgl_field_p, r_last_sgl_p, \
                SIZE_IN_BYTES(offsetof(struct barco_sgl_le_t, addr2))
    DMA_PHV2MEM_SETUP(barco_sgl_tuple1_len_update_null_addr2,
                      barco_sgl_tuple1_len_update_link,
                      r_sgl_field_p, dma_p2m_2)
    b           endif6
    nop
    
  .brcase BARCO_SGL_NUM_TUPLES_MAX

    b           endif6
    nop
  .brend
endsw0:

else6:
    
    // Now apply padding by adjusting tuple pair which could be:
    // tuple0/tuple1, or
    // tuple1/tuple2, or
    // tuple2/tuple0 (of the next adjacent SGL)

switch2:    
  .brbegin
    br          r_sgl_tuple_no[1:0]
    add         r_pad_buf_addr, SEQ_KIVEC5_PAD_BUF_ADDR, r0     // delay slot

  .brcase BARCO_SGL_TUPLE0

    // last_sgl_p->len0 = r_last_blk_len
    // last_sgl_p->addr1 = comp_pad_buf
    // last_sgl_p->len1 = r_pad_len
    // zero out the rest of last_sgl_p
    
    add         r_sgl_field_p, r_last_sgl_p, \
                SIZE_IN_BYTES(offsetof(struct barco_sgl_le_t, len0))
    phvwrpair   p.barco_sgl_tuple0_pad_pad_buf_addr, r_pad_buf_addr.dx, \
                p.barco_sgl_tuple0_pad_pad_len, r_pad_len.wx
    DMA_PHV2MEM_SETUP(barco_sgl_tuple0_pad_last_blk_len,
                      barco_sgl_tuple0_pad_link,
                      r_sgl_field_p, dma_p2m_2)
    b           endsw2
    phvwr       p.barco_sgl_tuple0_pad_last_blk_len, r_last_blk_len.wx  // delay slot

  .brcase BARCO_SGL_TUPLE1

    // last_sgl_p->len1 = r_last_blk_len
    // last_sgl_p->addr2 = comp_pad_buf
    // last_sgl_p->len2 = r_pad_len
    // zero out the rest of last_sgl_p
    
    add         r_sgl_field_p, r_last_sgl_p, \
                SIZE_IN_BYTES(offsetof(struct barco_sgl_le_t, len1))
    phvwrpair   p.barco_sgl_tuple1_pad_pad_buf_addr, r_pad_buf_addr.dx, \
                p.barco_sgl_tuple1_pad_pad_len, r_pad_len.wx
    DMA_PHV2MEM_SETUP(barco_sgl_tuple1_pad_last_blk_len,
                      barco_sgl_tuple1_pad_link,
                      r_sgl_field_p, dma_p2m_2)
    b           endsw2
    phvwr       p.barco_sgl_tuple1_pad_last_blk_len, r_last_blk_len.wx  // delay slot
    
  .brcase BARCO_SGL_TUPLE2

if8:    
    bcf         [c6], endif8
    add         r_sgl_field_p, r_last_sgl_p, \
                SIZE_IN_BYTES(offsetof(struct barco_sgl_le_t, len2))    // delay slot
    // last_sgl_p->len2 = r_last_blk_len
    // zero out the rest of last_sgl_p
    
    DMA_PHV2MEM_SETUP(barco_sgl_tuple2_len_update_last_blk_len,
                      barco_sgl_tuple2_len_update_link,
                      r_sgl_field_p, dma_p2m_2)
    b           endsw2
    phvwr       p.barco_sgl_tuple2_len_update_last_blk_len, r_last_blk_len.wx  // delay slot
endif8:
    
    // last_sgl_p->len2 = r_last_blk_len
    // (last_sgl_p + 1)->addr0 = comp_pad_buf
    // (last_sgl_p + 1)->len0 = r_pad_len
    // zero out the rest of (last_sgl_p + 1)
    
    DMA_PHV2MEM_SETUP(last_blk_len_len,
                      last_blk_len_len,
                      r_sgl_field_p, dma_p2m_2)
    phvwrpair   p.barco_sgl_tuple2_pad_pad_buf_addr, r_pad_buf_addr.dx, \
                p.barco_sgl_tuple2_pad_pad_len, r_pad_len.wx
                
    // Note: register r_last_sgl_p should not advance as it will be used for
    // storage_seq_comp_sgl_pad_only_xfer below. Only r_sgl_field_p should
    // advance to the next SGL and field.
     
    add         r_sgl_field_p, r_last_sgl_p, \
                SIZE_IN_BYTES(sizeof(struct barco_sgl_le_t)) + \
                SIZE_IN_BYTES(offsetof(struct barco_sgl_le_t, addr0))
    DMA_PHV2MEM_SETUP(barco_sgl_tuple2_pad_pad_buf_addr,
                      barco_sgl_tuple2_pad_link,
                      r_sgl_field_p, dma_p2m_3)
    b           endsw2
    phvwr       p.last_blk_len_len, r_last_blk_len.wx      // delay slot
    
  .brcase BARCO_SGL_NUM_TUPLES_MAX

    b           endsw2
    nop
  .brend
endsw2:

endif6:
    
possible_sgl_pdma_xfer:

    // PDMA compressed data and/or update CP header to dest comp buffer
    seq         c3, SEQ_KIVEC5_CP_HDR_UPDATE_EN, 0
    bbeq.c3     SEQ_KIVEC5_SGL_PDMA_EN, 0, possible_rate_limit
    seq         c4, SEQ_KIVEC5_SGL_UPDATE_EN, 1         // delay slot
    bbeq        SEQ_KIVEC5_SGL_PDMA_PAD_ONLY, 0, sgl_pdma_xfer_full
    
    // sgl_update_en and padding_en must have been set, the result of which
    // is referenced here for PDMA transfer of the pad data.
    
    seq.c4      c4, SEQ_KIVEC5_PADDING_EN, 1            // delay slot
    bcf         [!c4], pdma_pad_only_error
    nop
    LOAD_TABLE2_FOR_ADDR_PC_IMM(r_last_sgl_p, 
                                STORAGE_DEFAULT_TBL_LOAD_SIZE,
                                storage_seq_comp_sgl_pad_only_xfer)
    b           possible_rate_limit
    nop
    
sgl_pdma_xfer_full:
                                
    // PDMA compressed data to user buffers specified in SGL
    LOAD_TABLE1_FOR_ADDR_PC_IMM(SEQ_KIVEC2_SGL_PDMA_DST_ADDR, 
                                STORAGE_DEFAULT_TBL_LOAD_SIZE,
                                storage_seq_comp_sgl_pdma_xfer)
possible_rate_limit:

    // Use the newly calculated data length for rate limiting, if applicable.
    // General algorithm is as follows:
    // - if rate_limit_en
    //       r_src_len = pad_en ? data_len + pad_len : data_len
    //       r_rl_len = 0
    //       - if rate_limit_src_en
    //             r_rl_len = r_src_len
    //       - if rate_limit_dst_en
    //             if pdma_pad_only
    //                 r_rl_len += pad_len;
    //             else
    //                 r_rl_len += r_src_len

    add         r_src_len, l_data_len, r0
    bbeq        SEQ_KIVEC5_RATE_LIMIT_EN, 0, possible_barco_push
    add.c6      r_src_len, r_src_len, r_pad_len         // delay slot

    SEQ_RATE_LIMIT_ENABLE_CHECK(SEQ_KIVEC5_RATE_LIMIT_SRC_EN, c3)
    SEQ_RATE_LIMIT_DATA_LEN_LOAD_c(c3, r_src_len)
if10:    
    bbeq        SEQ_KIVEC5_RATE_LIMIT_DST_EN, 0, endif10
    SEQ_RATE_LIMIT_ENABLE_CHECK(SEQ_KIVEC5_SGL_PDMA_PAD_ONLY, c4) // delay slot
    SEQ_RATE_LIMIT_DATA_LEN_ADD_c(c4, r_pad_len)
    SEQ_RATE_LIMIT_DATA_LEN_ADD_c(!c4, r_src_len)
endif10:
    SEQ_RATE_LIMIT_SET_c(c0)
    
possible_barco_push:

    // If Barco ring push is applicable, execute table lock read
    // to get the current ring pindex. Note that this must be done
    // in the same stage as storage_seq_barco_entry_handler()
    // which is stage 3.
    bbeq        SEQ_KIVEC5_NEXT_DB_ACTION_BARCO_PUSH, 0, all_dma_complete
    nop
    SEQ_METRICS_VAL_SET(seq_hw_bytes, r_src_len)
    LOAD_TABLE_NO_LKUP_PC_IMM(0, storage_seq_barco_ring_pndx_read)
    
all_dma_complete:

    // Setup the start and end DMA pointers
    DMA_PTR_SETUP_e(dma_p2m_0_dma_cmd_pad,
                    dma_p2m_19_dma_cmd_eop,
                    p4_txdma_intr_dma_cmd_ptr)
comp_error:

    // If next_db_en and !stop_chain_on_error then ring_db
    seq         c5, SEQ_KIVEC5_NEXT_DB_EN, 1
    bbeq.c5     SEQ_KIVEC5_STOP_CHAIN_ON_ERROR, 0, possible_chain_alt_desc
    SEQ_METRICS_SET(hw_op_errs)                        // delay slot

    // override doorbell to raising an interrupt if possible
    LOAD_TABLE_NO_LKUP_PC_IMM(1, storage_seq_comp_db_intr_override)
    b           all_dma_complete
    phvwr	p.comp_hdr_cksum, r0                    // delay slot

possible_chain_alt_desc:

    // On compress error, possibly can still chain using an alternate set of
    // next-in-chain descriptors.

    tblwr.l	l_data_len, r0
    bbeq        SEQ_KIVEC5_CHAIN_ALT_DESC_ON_ERROR, 0, possible_sgl_pdma_alt_src
    sll         r_alt_desc_addr, SEQ_KIVEC4_BARCO_NUM_DESCS, \
                SEQ_KIVEC4_BARCO_DESC_SIZE      // delay slot
    add         r_alt_desc_addr, r_alt_desc_addr, SEQ_KIVEC4_BARCO_DESC_ADDR
    phvwr       p.seq_kivec4_barco_desc_addr, r_alt_desc_addr
    phvwr       p.seq_kivec10_alt_descs_select, 1
    tblwr.l	l_data_len, SEQ_KIVEC5_ALT_DATA_LEN
    SEQ_METRICS_SET(alt_descs_taken)
    
possible_sgl_pdma_alt_src:

    // On compress error, possibly can still do SGL PDMA using an alternate
    // source buffer address.

    bbeq        SEQ_KIVEC5_SGL_PDMA_ALT_SRC_ON_ERROR, 0, possible_rate_limit
    phvwr	p.comp_hdr_cksum, r0                    // delay slot
    tblwr.l	l_data_len, SEQ_KIVEC5_ALT_DATA_LEN
    SEQ_METRICS_SET(alt_bufs_taken)
    b           sgl_pdma_xfer_full
    phvwri      p.seq_kivec8_alt_buf_addr_en, 1         // delay slot
    
pdma_pad_only_error:

   // PDMA pad-only requires sgl_pad_en to also be enabled
   // (the sgl_pad_en operation provides the necessary SGL
   // for the calculation and resulting PDMA transfer of the pad data)
   
   SEQ_METRICS_SET(sgl_pad_only_errs)
   SEQ_COMP_SGL_PDMA_PAD_ONLY_ERROR_TRAP()
   b            possible_rate_limit
   nop


/*****************************************************************************
 * seq_xts_comp_len_update:
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct s3_tbl2_k k;
struct s3_tbl2_seq_xts_comp_len_update_d d;
struct phv_ p;

/*
 * Registers usage:
 * CAUTION: r1 is also implicitly used by LOAD_TABLE1_FOR_ADDR_PC_IMM()
 */
#define r_update_len                r1  // length to write into SGL tuple
#define r_last_blk_no               r2  // last block number
#define r_blk_boundary              r3  // 1 << blk_len_shift
#define r_last_blk_len              r4  // length of last block
#define r_data_len                  r5  // comp output data length
#define r_comp_desc_p               r6  // pointer to comp descriptor
#define r_num_blks                  r7  // number of blocks

/*
 * Registers reuse, pre calculations
 */
#define r_rl_len                    r_comp_desc_p   // rate limit length for SEQ_RATE_LIMIT_...()
 
/*
 * Registers reuse, post calculations
 */
#define r_last_sgl_p                r_comp_desc_p   // pointer to last SGL descriptor
#define r_field_p                   r_blk_boundary  // pointer to an SGL or desc field
#define r_desc_datain_len           r_num_blks      // descriptor datain_len
#define r_sgl_tuple_no              r_num_blks      // SGL tuple number
#define r_sgl_len_total             r_last_blk_no   // length total of all SGL tuple buffers

/*
 * Registers reuse, post update
 */
#define r_src_qaddr                 r_last_blk_len  // for SEQ_METRICS_TABLE_COMMIT
 
%%
    SEQ_METRICS_PARAMS()

storage_seq_xts_comp_len_update:

    // Compression length of 0 means 64K
    seq         c1, d.data_len, r0
    cmov        r_data_len, c1, \
                65536 - SIZE_IN_BYTES(sizeof(struct seq_comp_hdr_t)), d.data_len
    add         r_data_len, r_data_len, SIZE_IN_BYTES(sizeof(struct seq_comp_hdr_t))

    // We're assuming that descriptors submission will follow in a later stage;
    // otherwise there would be no reason to have requested a length update.

    SEQ_METRICS_VAL_SET(seq_hw_bytes, r_data_len)
if2:    
    bbeq        SEQ_KIVEC5XTS_RATE_LIMIT_EN, 0, endif2
    SEQ_RATE_LIMIT_ENABLE_CHECK(SEQ_KIVEC5XTS_RATE_LIMIT_SRC_EN, c3) // delay slot
    SEQ_RATE_LIMIT_DATA_LEN_LOAD_c(c3, r_data_len)
    SEQ_RATE_LIMIT_ENABLE_CHECK(SEQ_KIVEC5XTS_RATE_LIMIT_DST_EN, c4)
    SEQ_RATE_LIMIT_DATA_LEN_ADD_c(c4, r_data_len)
    SEQ_RATE_LIMIT_SET_c(c0)
endif2:

    // Preliminary calculations:
    // r_num_blks = (r_data_len + r_blk_boundary - 1) / r_blk_boundary)
    // r_last_blk_no = r_num_blks - 1
    // r_last_blk_len = r_data_len % r_blk_boundary
    
    sll         r_blk_boundary, 1, SEQ_KIVEC5XTS_BLK_BOUNDARY_SHIFT
    sub         r_blk_boundary, r_blk_boundary, 1
    add         r_num_blks, r_data_len, r_blk_boundary
    srl         r_num_blks, r_num_blks, SEQ_KIVEC5XTS_BLK_BOUNDARY_SHIFT
    sub         r_last_blk_no, r_num_blks, 1
    and         r_last_blk_len, r_data_len, r_blk_boundary

    // now update datain_len in compress descriptor
    add         r_comp_desc_p, SEQ_KIVEC7XTS_COMP_DESC_ADDR, r0
if4:    
    seq         c3, SEQ_KIVEC5XTS_DESC_VEC_PUSH_EN, 1
    phvwr.c3    p.seq_kivec4_barco_num_descs, r_num_blks
    bcf         [!c3], endif4
    add         r_desc_datain_len, r_data_len, r0       // delay slot

    // Case of vector of next-in-chain descriptors: find the last descriptor
    // and update its datain_len to r_last_blk_len
    
    add         r_comp_desc_p, r_comp_desc_p, r_last_blk_no, COMP_DESC_SIZE_SHIFT
    seq         c3, r_last_blk_len, r0
    add.c3      r_desc_datain_len, r_blk_boundary, 1
    add.!c3     r_desc_datain_len, r_last_blk_len, r0
endif4:

    // Do in-stage datain_len update in the descriptor.
    // This memory update must complete before storage_seq_barco_chain_action
    // transfers the decriptor(s) to Barco.
    add         r_field_p, r_comp_desc_p, \
                SIZE_IN_BYTES(offsetof(struct comp_desc_le_t, datain_len))
    memwr.hx    r_field_p, r_desc_datain_len            
    
    // Now, prepare to update length fields in the accompanying
    // SGL (or SGL vector), if any. These don't have any order
    // dependency with Barco descriptor transfer so we can use
    // PHV2MEM DMA.
    add         r_sgl_tuple_no, r0, r0
if6:    
    bbeq        SEQ_KIVEC5XTS_COMP_SGL_SRC_EN, 0, endif6
    add         r_last_sgl_p, SEQ_KIVEC7XTS_COMP_SGL_SRC_ADDR, r0 // delay slot
if8:    
    bbeq        SEQ_KIVEC5XTS_COMP_SGL_SRC_VEC_EN, 0, endif8
    add         r_update_len, r_data_len, r0    // delay slot

    // Note: each SGL tuple is expected to point to a block of size r_pad_boundary
    //
    // Calculate
    //   r_sgl_len_total = num_blks_per_sgl << blk_boundary_shift
    //   r_last_sgl_p = &comp_sgl_src_addr[r_data_len / r_sgl_len_total]
    //   r_sgl_tuple_no = (r_data_len % r_sgl_len_total) >> blk_boundary_shift
    //
    // Note: blk_boundary is a power of 2 but num_blks_per_sgl may not be 
    // (it either equals 1 or 3)

    seq         c3, SEQ_KIVEC5XTS_SGL_SPARSE_FORMAT_EN, 1
    sll.c3      r_sgl_len_total, 1, SEQ_KIVEC5XTS_BLK_BOUNDARY_SHIFT 
    sll.!c3     r_sgl_len_total, BARCO_SGL_NUM_TUPLES_MAX, SEQ_KIVEC5XTS_BLK_BOUNDARY_SHIFT 
    div         r_last_sgl_p, r_data_len, r_sgl_len_total
    add         r_last_sgl_p, SEQ_KIVEC7XTS_COMP_SGL_SRC_ADDR, \
                r_last_sgl_p, BARCO_SGL_DESC_SIZE_SHIFT
    mod         r_sgl_tuple_no, r_data_len, r_sgl_len_total
    srl         r_sgl_tuple_no, r_sgl_tuple_no, SEQ_KIVEC5XTS_BLK_BOUNDARY_SHIFT
    add         r_update_len, r_last_blk_len, r0
endif8:

    // Note: by this time we know that, since r_data_len is guaranteed
    // to be non-zero, a zero r_update_len means we need to "truncate" 
    // the SGL vector at the right spot.
if10:
    bne         r_update_len, r0, else10
    seq         c3, r_sgl_tuple_no, r0                          // delay slot
    
switch0:    
  .brbegin
    br          r_sgl_tuple_no[1:0]
    sub.c3      r_last_sgl_p, r_last_sgl_p, BARCO_SGL_DESC_SIZE // delay slot

  .brcase BARCO_SGL_TUPLE0

    // last_sgl_p->link = 0
    add         r_field_p, r_last_sgl_p, \
                SIZE_IN_BYTES(offsetof(struct barco_sgl_le_t, link))
    DMA_PHV2MEM_SETUP(barco_sgl_tuple2_len_update_link,
                      barco_sgl_tuple2_len_update_link,
                      r_field_p, dma_p2m_2)
    b           endif10
    nop

  .brcase BARCO_SGL_TUPLE1

    // zero out the rest of last_sgl_p starting from tuple1
    add         r_field_p, r_last_sgl_p, \
                SIZE_IN_BYTES(offsetof(struct barco_sgl_le_t, addr1))
    DMA_PHV2MEM_SETUP(barco_sgl_tuple0_len_update_null_addr1,
                      barco_sgl_tuple0_len_update_link,
                      r_field_p, dma_p2m_2)
    b           endif10
    nop
    
  .brcase BARCO_SGL_TUPLE2

    // zero out the rest of last_sgl_p starting from tuple2
    add         r_field_p, r_last_sgl_p, \
                SIZE_IN_BYTES(offsetof(struct barco_sgl_le_t, addr2))
    DMA_PHV2MEM_SETUP(barco_sgl_tuple1_len_update_null_addr2,
                      barco_sgl_tuple1_len_update_link,
                      r_field_p, dma_p2m_2)
    b           endif10
    nop
    
  .brcase BARCO_SGL_NUM_TUPLES_MAX

    b           endif10
    nop
  .brend
endsw0:

else10:

    // Now update length field in
    // tuple0 or tuple1, or tuple2
    
switch2:    
  .brbegin
    br          r_sgl_tuple_no[1:0]
    add         r_field_p, r_last_sgl_p, \
                SIZE_IN_BYTES(offsetof(struct barco_sgl_le_t, len0))    // delay slot
  .brcase BARCO_SGL_TUPLE0

    // last_sgl_p->len0 = r_update_len
    // zero out the rest of last_sgl_p
    
    DMA_PHV2MEM_SETUP(barco_sgl_tuple0_len_update_last_blk_len,
                      barco_sgl_tuple0_len_update_link,
                      r_field_p, dma_p2m_2)
    b           endsw2
    phvwr       p.barco_sgl_tuple0_len_update_last_blk_len, r_update_len.wx  // delay slot

  .brcase BARCO_SGL_TUPLE1

    // last_sgl_p->len1 = r_update_len
    // zero out the rest of last_sgl_p
    
    add         r_field_p, r_last_sgl_p, \
                SIZE_IN_BYTES(offsetof(struct barco_sgl_le_t, len1))
    DMA_PHV2MEM_SETUP(barco_sgl_tuple1_len_update_last_blk_len,
                      barco_sgl_tuple1_len_update_link,
                      r_field_p, dma_p2m_2)
    b           endsw2
    phvwr       p.barco_sgl_tuple1_len_update_last_blk_len, r_update_len.wx  // delay slot
    
  .brcase BARCO_SGL_TUPLE2

    // last_sgl_p->len2 = r_update_len
    // zero out the rest of last_sgl_p
    
    add         r_field_p, r_last_sgl_p, \
                SIZE_IN_BYTES(offsetof(struct barco_sgl_le_t, len2))
    DMA_PHV2MEM_SETUP(barco_sgl_tuple2_len_update_last_blk_len,
                      barco_sgl_tuple2_len_update_link,
                      r_field_p, dma_p2m_2)
    b           endsw2
    phvwr       p.barco_sgl_tuple2_len_update_last_blk_len, r_update_len.wx  // delay slot
    
  .brcase BARCO_SGL_NUM_TUPLES_MAX
  
    b           endsw2
    nop
  .brend
endsw2:
    
endif10:

endif6:

    // Relaunch stats commit for table 2
    SEQ_METRICS1_TABLE2_COMMIT(SEQ_KIVEC5XTS_SRC_QADDR)
    
    wrfence.e
    SEQ_METRICS_SET(len_updates)                        // delay slot


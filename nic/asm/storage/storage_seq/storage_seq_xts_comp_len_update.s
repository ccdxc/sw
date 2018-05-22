/*****************************************************************************
 * seq_xts_comp_len_update:
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

/*
 * Workaround for storage_seq_barco_chain_action mem2mem transfer of descriptor
 * occuring before phv2mem update of descriptor's datain_len.
 */
#define DESC_DATAIN_LEN_WRITE_IMMED     1


struct s3_tbl2_k k;
struct s3_tbl2_seq_xts_comp_len_update_d d;
struct phv_ p;

/*
 * Registers usage:
 */
#define r_last_blk_no               r1  // last block number
#define r_last_sgl_p                r2  // pointer to last SGL descriptor
#define r_blk_boundary              r3  // 1 << blk_len_shift
#define r_last_blk_len              r4  // length of last block
#define r_data_len                  r5  // comp output data length
#define r_comp_desc_p               r6  // pointer to comp descriptor
#define r_num_blks                  r7  // number of blocks

/*
 * Registers reuse, post calculations
 */
#define r_field_p                   r_blk_boundary  // pointer to an SGL or desc field
#define r_desc_datain_len           r_num_blks      // descriptor datain_len

%%

storage_seq_xts_comp_len_update:

    // Compression length of 0 means 64K
    seq         c1, d.data_len, r0
    cmov        r_data_len, c1, 65536 - SIZE_IN_BYTES(sizeof(struct seq_comp_hdr_t)), d.data_len
    add         r_data_len, r_data_len, SIZE_IN_BYTES(sizeof(struct seq_comp_hdr_t))

    // For PDMA, transfer length either comes from the supplied descriptor
    // (as stored in seq_kivec5xts_data_len by seq_xts_status_desc1_handler),
    // or from here. If the latter, we'll override seq_kivec5xts_data_len.
    seq         c2, SEQ_KIVEC5XTS_SGL_PDMA_LEN_FROM_DESC, 0
    phvwr.c2    p.seq_kivec5xts_data_len, r_data_len
    
    // Preliminary calculations:
    // r_num_blks = (r_data_len + r_blk_boundary - 1) / r_blk_boundary)
    // r_last_blk_no = r_num_blks - 1
    // r_last_blk_len = r_data_len % r_blk_boundary
    
    sll         r_blk_boundary, 1, SEQ_KIVEC5XTS_BLK_LEN_SHIFT
    sub         r_blk_boundary, r_blk_boundary, 1
    add         r_num_blks, r_data_len, r_blk_boundary
    srl         r_num_blks, r_num_blks, SEQ_KIVEC5XTS_BLK_LEN_SHIFT
    sub         r_last_blk_no, r_num_blks, 1
    and         r_last_blk_len, r_data_len, r_blk_boundary

    // now update datain_len in compress descriptor
    add         r_comp_desc_p, SEQ_KIVEC7XTS_COMP_DESC_ADDR, r0
    seq         c3, SEQ_KIVEC5XTS_DESC_VEC_PUSH_EN, 1
    phvwr.c3    p.seq_kivec4_barco_num_descs, r_num_blks
    
#ifdef DESC_DATAIN_LEN_WRITE_IMMED
    
if0:    
    bcf         [!c3], endif0
    add         r_desc_datain_len, r_data_len, r0       // delay slot

    // Case of vector of next-in-chain descriptors: find the last descriptor
    // and update its datain_len to r_last_blk_len
    
    add         r_comp_desc_p, r_comp_desc_p, r_last_blk_no, COMP_DESC_SIZE_SHIFT
    add         r_desc_datain_len, r_last_blk_len, r0

endif0:

    // Assume threshold_len does not need to be updated
    add         r_field_p, r_comp_desc_p, \
                SIZE_IN_BYTES(offsetof(struct comp_desc_le_t, datain_len))
    // Note: datain_len is 16 bits wide so 65536 would again bea
    // truncated back to 0.
    memwr.hx    r_field_p, r_desc_datain_len

#else

if0:    
    bcf         [!c3], endif0
    phvwr       p.comp_datain_len_len, r_data_len.hx    // delay slot

    // Case of vector of next-in-chain descriptors: find the last descriptor
    // and update its datain_len to r_last_blk_len
    
    add         r_comp_desc_p, r_comp_desc_p, r_last_blk_no, COMP_DESC_SIZE_SHIFT
    phvwr       p.comp_datain_len_len, r_last_blk_len.hx

endif0:

    // Assume threshold_len does not need to be updated
    add         r_field_p, r_comp_desc_p, \
                SIZE_IN_BYTES(offsetof(struct comp_desc_le_t, datain_len))

    // Note: p.comp_datain_len_len is 16 bits wide so 65536 would again
    // have been truncated back to 0 (in an earlier phvwr).
    DMA_PHV2MEM_SETUP_ADDR64(comp_datain_len_len, comp_datain_len_len,
                             r_field_p, dma_p2m_2)
#endif
                             
    // Now, prepare to update length field in the accompanying
    // SGL (or SGL vector), if any.  
if1:    
    bbeq        SEQ_KIVEC5XTS_COMP_SGL_SRC_EN, 0, endif1
    add         r_last_sgl_p, SEQ_KIVEC7XTS_COMP_SGL_SRC_ADDR, r0 // delay slot
    
if2:    
    bbeq        SEQ_KIVEC5XTS_COMP_SGL_SRC_VEC_EN, 0, endif2
    phvwr       p.comp_last_blk_len_len, r_data_len.wx    // delay slot
    
    add         r_last_sgl_p, r_last_sgl_p, r_last_blk_no, BARCO_SGL_DESC_SIZE_SHIFT
    phvwr       p.comp_last_blk_len_len, r_last_blk_len.wx

endif2:
    add         r_field_p, r_last_sgl_p, \
                SIZE_IN_BYTES(offsetof(struct barco_sgl_le_t, len0))
    DMA_PHV2MEM_SETUP_ADDR64(comp_last_blk_len_len, comp_last_blk_len_len,
                             r_field_p, dma_p2m_3)
    // Terminate the SGL vector at the current descriptor
    add         r_field_p, r_last_sgl_p, \
                SIZE_IN_BYTES(offsetof(struct barco_sgl_le_t, link))
    DMA_PHV2MEM_SETUP_ADDR64(null_addr_addr, null_addr_addr,
                             r_field_p, dma_p2m_4)
endif1:

    CLEAR_TABLE2_e
    

/*****************************************************************************
 *  seq_comp_aol_pad_handler
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s3_tbl3_k k;
struct phv_ p;

/*
 * AOL rearranged to little-endian layout
 */
struct barco_aol_le_t {
    rsvd : 64;
    next_addr : 64;
    L2 : 32;
    O2 : 32;
    A2 : 64;
    L1 : 32;
    O1 : 32;
    A1 : 64;
    L0 : 32;
    O0 : 32;
    A0 : 64;
};

/*
 * Registers usage:
 */
#define r_last_blk_no               r1  // last block number
#define r_last_aol_p                r2  // pointer to last AOL descriptor
#define r_aol_field_p               r3  // pointer to an AOL field

%%

storage_seq_comp_aol_pad_handler:

    sub         r_last_blk_no, SEQ_KIVEC3_NUM_BLKS, 1
    add         r_last_aol_p, SEQ_KIVEC6_AOL_SRC_VEC_ADDR, \
                r_last_blk_no, BARCO_AOL_DESC_SIZE_SHIFT
                
    // Set up DMA for the following:
    // last_src_aol_p->l0 = last_blk_len
    // last_src_aol_p->a1 = pad_buf_addr
    // last_src_aol_p->l1 = pad_len
    // last_src_aol_p->next_addr = 0
    
    add         r_aol_field_p, r_last_aol_p, \
                SIZE_IN_BYTES(offsetof(struct barco_aol_le_t, L0))      // delay slot
    DMA_PHV2MEM_SETUP_ADDR64(comp_last_blk_len_len, comp_last_blk_len_len,
                             r_aol_field_p, dma_p2m_5)
    
if0:
    seq         c1, SEQ_KIVEC3_PAD_LEN, r0
    bcf         [c1], endif0
    
    add         r_aol_field_p, r_last_aol_p, \
                SIZE_IN_BYTES(offsetof(struct barco_aol_le_t, A1))      // delay slot
    DMA_PHV2MEM_SETUP_ADDR64(pad_buf_addr_addr, pad_buf_addr_addr,
                             r_aol_field_p, dma_p2m_6)
    
    add         r_aol_field_p, r_last_aol_p, \
                SIZE_IN_BYTES(offsetof(struct barco_aol_le_t, L1))
    DMA_PHV2MEM_SETUP_ADDR64(comp_pad_len_len, comp_pad_len_len,
                             r_aol_field_p, dma_p2m_7)
    
endif0:
    
    add         r_aol_field_p, r_last_aol_p, \
                SIZE_IN_BYTES(offsetof(struct barco_aol_le_t, next_addr))
    DMA_PHV2MEM_SETUP_ADDR64(null_addr_addr, null_addr_addr,
                             r_aol_field_p, dma_p2m_8)
    
    // Set up DMA for the following:
    // last_dst_aol_p->next_addr = 0
    add         r_last_aol_p, SEQ_KIVEC6_AOL_DST_VEC_ADDR, \
                r_last_blk_no, BARCO_AOL_DESC_SIZE_SHIFT
    add         r_aol_field_p, r_last_aol_p, \
                SIZE_IN_BYTES(offsetof(struct barco_aol_le_t, next_addr))
    DMA_PHV2MEM_SETUP_ADDR64(null_addr_addr, null_addr_addr,
                             r_aol_field_p, dma_p2m_9)
    CLEAR_TABLE3_e


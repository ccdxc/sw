/*****************************************************************************
 *  storage_seq_comp_aol_pad_handler
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s3_tbl3_k k;
struct phv_ p;

/*
 * Registers usage:
 * CAUTION: r1 is also implicitly used by LOAD_TABLE1_FOR_ADDR_PC_IMM()
 */
#define r_last_blk_no               r3  // last block number
#define r_last_aol_p                r4  // pointer to last AOL descriptor
#define r_aol_field_p               r5  // pointer to an AOL field
#define r_last_blk_len              r6  // length of last block
#define r_src_qaddr                 r7  // for SEQ_METRICS_TABLE_COMMIT

%%
    SEQ_METRICS_PARAMS()

storage_seq_comp_aol_pad_handler:

    // Note:
    // - AOL is always in sparse format
    // - By this time we know that crypto length is guaranteed to be non-zero
    //   (because we got here from CP/DC where 0 would have been converted to
    //    a max length).
    //
    // If last_blk_len is 0 (a modulo of block size), we can assume it was
    // a multiple of block size and, due to sparse format, we can convert it
    // to block size.
    seq         c3, SEQ_KIVEC3_LAST_BLK_LEN, r0
    sll.c3      r_last_blk_len, 1, SEQ_KIVEC3_PAD_BOUNDARY_SHIFT
    add.!c3     r_last_blk_len, SEQ_KIVEC3_LAST_BLK_LEN, r0
    
    // Set up memwr for the following:
    // last_src_aol_p->l0 = last_blk_len
    // last_src_aol_p->a1 = pad_buf_addr
    // last_src_aol_p->l1 = pad_len
    // last_src_aol_p->next_addr = 0
    //
    // Note: if padding_en=0, both pad_buf_addr and pad_len would
    // have been set to 0 from a previous stage.
                
    sub         r_last_blk_no, SEQ_KIVEC3_NUM_BLKS, 1
    add         r_last_aol_p, SEQ_KIVEC6_AOL_SRC_VEC_ADDR, \
                r_last_blk_no, BARCO_AOL_DESC_SIZE_SHIFT
    add         r_aol_field_p, r_last_aol_p, \
                SIZE_IN_BYTES(offsetof(struct barco_aol_le_t, L0))
    memwr.wx    r_aol_field_p, r_last_blk_len
    add         r_aol_field_p, r_last_aol_p, \
                SIZE_IN_BYTES(offsetof(struct barco_aol_le_t, A1))
    memwr.dx    r_aol_field_p, SEQ_KIVEC5_PAD_BUF_ADDR
    add         r_aol_field_p, r_last_aol_p, \
                SIZE_IN_BYTES(offsetof(struct barco_aol_le_t, L1))
    memwr.wx    r_aol_field_p, SEQ_KIVEC3_PAD_LEN
    add         r_aol_field_p, r_last_aol_p, \
                SIZE_IN_BYTES(offsetof(struct barco_aol_le_t, next_addr))
    memwr.dx    r_aol_field_p, r0
                    
    // Set up memwr for the following:
    // last_dst_aol_p->l0 = last_blk_len + pad_len
    //   Note: pad_len would have been zero if padding_en=0 or
    //         when last_blk_len iss exactly block size.
    // last_dst_aol_p->next_addr = 0
    
    add         r_last_aol_p, SEQ_KIVEC6_AOL_DST_VEC_ADDR, \
                r_last_blk_no, BARCO_AOL_DESC_SIZE_SHIFT
    add         r_aol_field_p, r_last_aol_p, \
                SIZE_IN_BYTES(offsetof(struct barco_aol_le_t, L0))
    add         r_last_blk_len, r_last_blk_len, SEQ_KIVEC3_PAD_LEN
    memwr.wx    r_aol_field_p, r_last_blk_len
    add         r_aol_field_p, r_last_aol_p, \
                SIZE_IN_BYTES(offsetof(struct barco_aol_le_t, next_addr))
    memwr.dx    r_aol_field_p, r0

    // Relaunch stats commit for table 3
    SEQ_METRICS2_TABLE3_COMMIT(SEQ_KIVEC5_SRC_QADDR)

    wrfence.e
    SEQ_METRICS_SET(aol_update_reqs)                       // delay slot
    

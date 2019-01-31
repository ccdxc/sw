/*****************************************************************************
 *  seq_xts_status_handler:
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s2_tbl_k k;
struct s2_tbl_seq_xts_status_handler_d d;
struct phv_ p;

/*
 * Registers usage:
 * CAUTION: r1 is also implicitly used by LOAD_TABLE1_FOR_ADDR_PC_IMM()
 */
#define r_rl_len                    r3  // rate limit length for SEQ_RATE_LIMIT_...()
#define r_src_len                   r6
#define r_src_qaddr                 r7  // qstate address
 
%%
   .param storage_seq_barco_ring_pndx_read
   .param storage_seq_xts_comp_len_update
   .param storage_seq_xts_sgl_pdma_xfer
   .param storage_seq_xts_db_intr_override

storage_seq_xts_status_handler:

   seq          c4, d.err, r0
   bcf          [!c4], xts_error
   CLEAR_TABLE0 // delay slot

    // Launch read of compression header from the decrypted data buffer
    // (if applicable) so next-in-chain descriptor and SGLs
    // can be updated accordingly.
    bbeq        SEQ_KIVEC5XTS_COMP_LEN_UPDATE_EN, 0, possible_sgl_pdma_xfer
    nop
    LOAD_TABLE2_FOR_ADDR_PC_IMM(SEQ_KIVEC2XTS_DECR_BUF_ADDR,
                                STORAGE_TBL_LOAD_SIZE_64_BITS,
                                storage_seq_xts_comp_len_update)

possible_sgl_pdma_xfer:

    // PDMA decrypted data to user buffers specified in SGL
    bbeq        SEQ_KIVEC5XTS_SGL_PDMA_EN, 0, possible_rate_limit
    phvwr       p.seq_kivec3xts_decr_buf_addr, SEQ_KIVEC2XTS_DECR_BUF_ADDR // delay slot
    
    // PDMA compressed data to user buffers specified in SGL
    LOAD_TABLE1_FOR_ADDR_PC_IMM(SEQ_KIVEC2XTS_SGL_PDMA_DST_ADDR, 
                                STORAGE_DEFAULT_TBL_LOAD_SIZE,
                                storage_seq_xts_sgl_pdma_xfer)
possible_rate_limit:

    // General algorithm is as follows:
    // - if rate_limit_en
    //       r_src_len = data_len
    //       r_rl_len = 0
    //       - if rate_limit_src_en
    //             r_rl_len = r_src_len
    //       - if rate_limit_dst_en
    //             r_rl_len += r_src_len
    
    bbeq        SEQ_KIVEC5XTS_RATE_LIMIT_EN, 0, possible_barco_push
    add         r_src_len, SEQ_KIVEC5XTS_DATA_LEN, r0   // delay slot
    
    // set rate limit with known value here;
    // Note: storage_seq_xts_comp_len_update can change it in the next stage.
    SEQ_RATE_LIMIT_ENABLE_CHECK(SEQ_KIVEC5XTS_RATE_LIMIT_SRC_EN, c3)
    SEQ_RATE_LIMIT_DATA_LEN_LOAD_c(c3, r_src_len)
    SEQ_RATE_LIMIT_ENABLE_CHECK(SEQ_KIVEC5XTS_RATE_LIMIT_DST_EN, c4)
    SEQ_RATE_LIMIT_DATA_LEN_ADD_c(c4, r_src_len)
    SEQ_RATE_LIMIT_SET_c(c0)

possible_barco_push:

    // if Barco ring push is applicable, execute table lock read
    // to get the current ring pindex. Note that this must be done
    // in the same stage as storage_seq_barco_entry_handler()
    // which is stage 3.
    bbeq	SEQ_KIVEC5XTS_NEXT_DB_ACTION_BARCO_PUSH, 0, all_dma_complete
    nop
    SEQ_METRICS_VAL_SET(seq_hw_bytes, r_src_len)
    LOAD_TABLE_NO_LKUP_PC_IMM(0, storage_seq_barco_ring_pndx_read)
   
all_dma_complete:

   // Setup the start and end DMA pointers
   DMA_PTR_SETUP_e(dma_p2m_0_dma_cmd_pad,
                   dma_p2m_19_dma_cmd_eop,
                   p4_txdma_intr_dma_cmd_ptr)

xts_error:

   // if next_db_en and !stop_chain_on_error then ring_db
   seq          c5, SEQ_KIVEC5XTS_NEXT_DB_EN, 1
   bbeq.c5      SEQ_KIVEC5XTS_STOP_CHAIN_ON_ERROR, 0, possible_rate_limit
   SEQ_METRICS_SET(hw_op_errs)                          // delay slot

   // override doorbell to raising an interrupt if possible
   LOAD_TABLE_NO_LKUP_PC_IMM(1, storage_seq_xts_db_intr_override)
   b            all_dma_complete
   nop


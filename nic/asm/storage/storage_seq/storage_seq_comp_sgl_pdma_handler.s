/*****************************************************************************
 *  seq_comp_sgl_pdma_xfer: Parse the destination SGL and DMA the status,
 *                          data (if status was success) and set the interrupt.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s3_tbl1_k k;
struct s3_tbl1_seq_comp_sgl_handler_d d;
struct phv_ p;

/*
 * Registers usage
 */
#define r_next_dma_cmd_ptr          r1  // next DMA command pointer
#define r_src_len                   r2  // length of data source
#define r_xfer_len                  r3  // current transfer length
#define r_src_addr                  r4  // source address
#define r_dst_addr                  r5  // destination address
#define r_sgl_len                   r6  // current length of an SGL entry

/*
 * Registers reuse, post source transfer completion
 */
#define r_pad_len                   r_src_len // padding length
 
%%

storage_seq_comp_sgl_pdma_xfer:

   // Store the data length into r_src_len accouting for the fact that 0 => 64K
   add          r_src_len, SEQ_KIVEC5_DATA_LEN, r0

   // Setup the compression data buffer DMA based on flat source buffer 
   // and destination SGL.The macro processes one SGL entry and branches
   // if xfer is complete at any point.
   // Notes: These GPRs are used for input/output to/from this macro
   //  1. r_src_len stores the running count of data remaining to be xfered
   //  2. r_src_addr stores the offeset of the source data buffer from where
   //     the current xfer is to be done.
   //
   // Note that when next_db_action_barco_push is set, chances are some sort
   // of padding would also be required which took up some number of TxDMA
   // decriptors. So we will be limited with the number of SGLs we can process here.
   //
   // Alternatively, when next_db_action_barco_push is *not* set, the only padding
   // that would apply is for the case of pad-only transfer, which would be handled
   // entirely in storage_seq_comp_sgl_pad_only_xfer().
   
   bbeq         SEQ_KIVEC5_NEXT_DB_ACTION_BARCO_PUSH, 1, limited_sgl_case
   add		r_src_addr, SEQ_KIVEC3_FLAT_BUF_ADDR, r0        // delay slot

all_sgl_plus_padding_case:
   
   // Can process the entire PDMA SGL here which holds 4 addr/len pairs,
   // plus padding
   
   COMP_SGL_DMA(SEQ_DMA_COMP_CHAIN_SGL_PDMA_QUAD_M2M_SRC0,
                SEQ_DMA_COMP_CHAIN_SGL_PDMA_QUAD_M2M_DST0,
                PHV_DMA_CMD_START_OFFSET(SEQ_DMA_COMP_CHAIN_SGL_PDMA_QUAD_M2M_SRC1),
                d.addr0, d.len0, possible_padding_apply)
   COMP_SGL_DMA(SEQ_DMA_COMP_CHAIN_SGL_PDMA_QUAD_M2M_SRC1,
                SEQ_DMA_COMP_CHAIN_SGL_PDMA_QUAD_M2M_DST1,
                PHV_DMA_CMD_START_OFFSET(SEQ_DMA_COMP_CHAIN_SGL_PDMA_QUAD_M2M_SRC2),
                d.addr1, d.len1, possible_padding_apply)
   COMP_SGL_DMA(SEQ_DMA_COMP_CHAIN_SGL_PDMA_QUAD_M2M_SRC2,
                SEQ_DMA_COMP_CHAIN_SGL_PDMA_QUAD_M2M_DST2,
                PHV_DMA_CMD_START_OFFSET(SEQ_DMA_COMP_CHAIN_SGL_PDMA_QUAD_M2M_SRC3),
                d.addr2, d.len2, possible_padding_apply)
   COMP_SGL_DMA(SEQ_DMA_COMP_CHAIN_SGL_PDMA_QUAD_M2M_SRC3,
                SEQ_DMA_COMP_CHAIN_SGL_PDMA_QUAD_M2M_DST3,
                PHV_DMA_CMD_START_OFFSET(SEQ_DMA_COMP_CHAIN_SGL_PDMA_QUAD_M2M_SRC4),
                d.addr3, d.len3, possible_padding_apply)

src_len_remain_check:

   // Catch any driver errors here for debugging, i.e., driver did not 
   // provision the SGL correctly relative to comp output data length.
   bne          r_src_len, r0, pdma_xfer_error
   nop
      
possible_padding_apply:

   // Apply padding to the current SGL addressed by r_dst_addr if applicable.
   // The remaining length in r_sgl_len must be >= r_pad_len.
   
   add          r_pad_len, SEQ_KIVEC3_PAD_LEN, r0
   beq          r_pad_len, r0, exit
   nop
   beq          r_next_dma_cmd_ptr, r0, pdma_xfer_error
   nop
   blt          r_sgl_len, r_pad_len, pdma_xfer_error
   nop

   // MEM2MEM source is pad buffer, assumed to be a 34-bit HBM address
   DMA_MEM2MEM_PTR_SETUP_ADDR34(CAPRI_DMA_M2M_TYPE_SRC,
                                SEQ_KIVEC3_PAD_BUF_ADDR, r_pad_len)
   
   // MEM2MEM destination
   // Note: PHV flit memory is in big endian layout, so the next adjacent TxDMA
   // descriptor is at a LOWER address! Hence, the subi instruction below.
   
   subi         r_next_dma_cmd_ptr, r_next_dma_cmd_ptr, sizeof(DMA_CMD_MEM2MEM_T)
   DMA_MEM2MEM_PTR_SETUP_ADDR(CAPRI_DMA_M2M_TYPE_DST,
                              r_dst_addr, r_pad_len)
   
exit:
   CLEAR_TABLE1_e

limited_sgl_case:
   // Only has enough dma_m2m for one, two, or three transfers
   // because an AOL pad action and/or SGL pad action have occupied
   // some of the DMA descriptors. So, try the best we could here.
   bbeq         SEQ_KIVEC5_AOL_PAD_EN, 0, alt1_quad_sgl_case
   nop
   bbeq         SEQ_KIVEC5_STATUS_DMA_EN, 0, possible_alt0_quad_sgl_case
   nop
   bbeq         SEQ_KIVEC5_SGL_PAD_HASH_EN, 0, alt0_quad_sgl_case
   nop
   
   // Can process only 1 addr/len pair, plus padding
   
   COMP_SGL_DMA(SEQ_DMA_COMP_CHAIN_SGL_PDMA_DOUBLE_M2M_SRC0,
                SEQ_DMA_COMP_CHAIN_SGL_PDMA_DOUBLE_M2M_DST0,
                PHV_DMA_CMD_START_OFFSET(SEQ_DMA_COMP_CHAIN_SGL_PDMA_DOUBLE_M2M_SRC1),
                d.addr0, d.len0, possible_padding_apply)
   b            src_len_remain_check
   nop

alt1_quad_sgl_case:
   bbeq         SEQ_KIVEC5_SGL_PAD_HASH_EN, 0, all_sgl_plus_padding_case
   nop
   
   // Can process 3 addr/len pairs, plus padding
   
   COMP_SGL_DMA(SEQ_DMA_COMP_CHAIN_SGL_PDMA_ALT1_QUAD_M2M_SRC0,
                SEQ_DMA_COMP_CHAIN_SGL_PDMA_ALT1_QUAD_M2M_DST0,
                PHV_DMA_CMD_START_OFFSET(SEQ_DMA_COMP_CHAIN_SGL_PDMA_ALT1_QUAD_M2M_SRC1),
                d.addr0, d.len0, possible_padding_apply)
   COMP_SGL_DMA(SEQ_DMA_COMP_CHAIN_SGL_PDMA_ALT1_QUAD_M2M_SRC1,
                SEQ_DMA_COMP_CHAIN_SGL_PDMA_ALT1_QUAD_M2M_DST1,
                PHV_DMA_CMD_START_OFFSET(SEQ_DMA_COMP_CHAIN_SGL_PDMA_ALT1_QUAD_M2M_SRC2),
                d.addr1, d.len1, possible_padding_apply)
   COMP_SGL_DMA(SEQ_DMA_COMP_CHAIN_SGL_PDMA_ALT1_QUAD_M2M_SRC2,
                SEQ_DMA_COMP_CHAIN_SGL_PDMA_ALT1_QUAD_M2M_DST2,
                PHV_DMA_CMD_START_OFFSET(SEQ_DMA_COMP_CHAIN_SGL_PDMA_ALT1_QUAD_M2M_SRC3),
                d.addr2, d.len2, possible_padding_apply)
   b            src_len_remain_check
   nop
                
possible_alt0_quad_sgl_case:
   bbeq         SEQ_KIVEC5_SGL_PAD_HASH_EN, 0, alt0_quad_sgl_case
   nop

   // Can process 2 addr/len pairs, plus padding
   
   COMP_SGL_DMA(SEQ_DMA_COMP_CHAIN_SGL_PDMA_TRIPLE_M2M_SRC0,
                SEQ_DMA_COMP_CHAIN_SGL_PDMA_TRIPLE_M2M_DST0,
                PHV_DMA_CMD_START_OFFSET(SEQ_DMA_COMP_CHAIN_SGL_PDMA_TRIPLE_M2M_SRC1),
                d.addr0, d.len0, possible_padding_apply)
   COMP_SGL_DMA(SEQ_DMA_COMP_CHAIN_SGL_PDMA_TRIPLE_M2M_SRC1,
                SEQ_DMA_COMP_CHAIN_SGL_PDMA_TRIPLE_M2M_DST1,
                PHV_DMA_CMD_START_OFFSET(SEQ_DMA_COMP_CHAIN_SGL_PDMA_TRIPLE_M2M_SRC2),
                d.addr1, d.len1, possible_padding_apply)
   b            src_len_remain_check
   nop
   
alt0_quad_sgl_case:

   // Can process 3 addr/len pairs, plus padding
   
   COMP_SGL_DMA(SEQ_DMA_COMP_CHAIN_SGL_PDMA_ALT0_QUAD_M2M_SRC0,
                SEQ_DMA_COMP_CHAIN_SGL_PDMA_ALT0_QUAD_M2M_DST0,
                PHV_DMA_CMD_START_OFFSET(SEQ_DMA_COMP_CHAIN_SGL_PDMA_ALT0_QUAD_M2M_SRC1),
                d.addr0, d.len0, possible_padding_apply)
   COMP_SGL_DMA(SEQ_DMA_COMP_CHAIN_SGL_PDMA_ALT0_QUAD_M2M_SRC1,
                SEQ_DMA_COMP_CHAIN_SGL_PDMA_ALT0_QUAD_M2M_DST1,
                PHV_DMA_CMD_START_OFFSET(SEQ_DMA_COMP_CHAIN_SGL_PDMA_ALT0_QUAD_M2M_SRC2),
                d.addr1, d.len1, possible_padding_apply)
   COMP_SGL_DMA(SEQ_DMA_COMP_CHAIN_SGL_PDMA_ALT0_QUAD_M2M_SRC2,
                SEQ_DMA_COMP_CHAIN_SGL_PDMA_ALT0_QUAD_M2M_DST2,
                PHV_DMA_CMD_START_OFFSET(SEQ_DMA_COMP_CHAIN_SGL_PDMA_ALT0_QUAD_M2M_SRC3),
                d.addr2, d.len2, possible_padding_apply)
   b            src_len_remain_check
   nop
                
pdma_xfer_error:
   SEQ_COMP_SGL_PDMA_XFER_ERROR_TRAP()
   b            exit
   nop



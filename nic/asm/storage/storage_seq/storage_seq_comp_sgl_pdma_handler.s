/*****************************************************************************
 *  seq_comp_sgl_pdma_xfer: Parse the destination SGL and DMA the data from
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s3_tbl1_k k;
struct s3_tbl1_seq_comp_sgl_pdma_xfer_d d;
struct phv_ p;

/*
 * Registers usage
 */
#define r_curr_dma_cmd_ptr          r1  // pointer to current TxDMA descriptor in flit
#define r_src_len                   r2  // length of data source
#define r_xfer_len                  r3  // current transfer length
#define r_src_addr                  r4  // source address
#define r_dst_addr                  r5  // destination address
#define r_sgl_tuple_p               r6  // pointer to the current SGL entry
#define r_last_dma_cmd_ptr          r7  // pointer to last TxDMA descriptor in flit

/*
 * Registers reuse, post 1st series of PDMA transfer
 */
#define r_sgl_rem_len               r_src_len
 
%%

storage_seq_comp_sgl_pdma_xfer:

   //  r_src_len stores the running count of data remaining to be xfered
   //  r_src_addr stores the offeset of the source data buffer from where
   //  the current xfer is to be done.
   add          r_src_len, SEQ_KIVEC5_DATA_LEN, r0
   add		r_src_addr, SEQ_KIVEC3_COMP_BUF_ADDR, r0

   /*
    * VERY IMPORTANT NOTE: mem2mem descriptors work in adjacent pair and the
    * pair must not cross flit boundary. P4+ code sets up PHV space which 
    * guarantees that a valid mem2mem pair always starts with an even numbered ID.
    *
    * For example: dma_m2m_2/dma_m2m_3 would be a valid pair, 
    *              but dma_m2m_7/dma_m2m_8 would not necessarily be adjacent.
    *
    * Currently it is known that dma_m2m_0/dma_m2m_1 are in one flit, and
    * all the subsequent mem2mem quads are in succeeding flits.
    *
    * The following is the initial flit where the first set of mem2mem
    * descriptors are available for PDMA use. Note that the follow on macro
    * invocations can advance into one or more subsequent flits!
    */
   CAPRI_FLIT_DMA_PTR_INITIAL(dma_m2m_6, dma_m2m_9)

   /*
    * Set up the initial d-vector chain_sgl_pdma tuple pointer in
    * r_sgl_tuple_p, for use by tblrdp/tblwrp. 
    */
   CAPRI_CHAIN_SGL_PDMA_TUPLE_INITIAL()
   
   CHAIN_SGL_PDMA_PTR(inner_label0, inner_label1, 
                      possible_padding_apply, pdma_xfer_error)
   CAPRI_CHAIN_SGL_PDMA_TUPLE_ADVANCE()
   
   CHAIN_SGL_PDMA_PTR(inner_label2, inner_label3, 
                      possible_padding_apply, pdma_xfer_error)
   CAPRI_CHAIN_SGL_PDMA_TUPLE_ADVANCE()
   
   CHAIN_SGL_PDMA_PTR(inner_label4, inner_label5, 
                      possible_padding_apply, pdma_xfer_error)
   CAPRI_CHAIN_SGL_PDMA_TUPLE_ADVANCE()
   CHAIN_SGL_PDMA_PTR(inner_label6, inner_label7,
                      possible_padding_apply, pdma_xfer_error)
   
   // Catch any driver errors here for debugging, i.e., driver did not 
   // provision the SGL correctly relative to comp output data length.
   bne          r_src_len, r0, pdma_xfer_error
   nop
      
possible_padding_apply:

   // Apply padding to the remaining area in the current SGL.
   // Note that when the last CHAIN_SGL_PDMA above exited to this point,
   // r_xfer_len contains the remaining length in the current SGL.
   //
   seq          c1, SEQ_KIVEC3_PAD_LEN, r0
   bcf          [c1], exit
   
   // CAUTION: Due to registers shortage, r_sgl_rem_len is equated
   // with r_src_len so only one of them can be in use at a time in
   // the computations below.
   
   tblrdp.wx    r_sgl_rem_len, r_sgl_tuple_p, \
                CAPRI_TBLRWP_FIELD_OP(chain_sgl_pdma_tuple_t, len) // delay slot
   sub          r_sgl_rem_len, r_sgl_rem_len, r_xfer_len
   
   slt          c2, r_sgl_rem_len, SEQ_KIVEC3_PAD_LEN
   cmov         r_xfer_len, c2, r_sgl_rem_len, SEQ_KIVEC3_PAD_LEN 
   
if0:
   beq          r_xfer_len, r0, endif0
   add		r_src_addr, SEQ_KIVEC5_PAD_BUF_ADDR, r0 // delay slot

   DMA_MEM2MEM_PTR_SETUP_ADDR(CAPRI_DMA_M2M_TYPE_SRC,
                              r_src_addr, r_xfer_len)
if1:                              
   CAPRI_FLIT_DMA_PTR_ADVANCE(endif1)
endif1:   
   DMA_MEM2MEM_PTR_SETUP_ADDR(CAPRI_DMA_M2M_TYPE_DST,
                              r_dst_addr, r_xfer_len)
   CAPRI_FLIT_DMA_PTR_ADVANCE(endif0)
endif0:
   
   sub          r_src_len, SEQ_KIVEC3_PAD_LEN, r_xfer_len
   beq          r_src_len, r0, exit
   add          r_src_addr, r_src_addr, r_xfer_len  // delay slot

   // Transfer the remaining pad data which must fit in the
   // next SGL entry
   CAPRI_CHAIN_SGL_PDMA_TUPLE_ADVANCE()
   CHAIN_SGL_PDMA_PTR(inner_label8, inner_label9,
                      exit, pdma_xfer_error)
   
   // Catch any driver errors here for debugging, i.e., driver did not 
   // provision the SGL correctly relative to padding length
   bne          r_src_len, r0, pdma_xfer_error
   nop
      
exit:
   CLEAR_TABLE1_e

pdma_xfer_error:
   SEQ_COMP_SGL_PDMA_XFER_ERROR_TRAP()
   b            exit
   nop



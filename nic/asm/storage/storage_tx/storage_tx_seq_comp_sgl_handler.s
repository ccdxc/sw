/*****************************************************************************
 *  seq_comp_sgl_handler: Parse the destination SGL and DMA the status,
 *                        data (if status was success) and set the interrupt.
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

storage_tx_seq_comp_sgl_handler_start:

   // Store the data length into r_src_len accouting for the fact that 0 => 64K
   add          r_src_len, STORAGE_KIVEC5_DATA_LEN, r0

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
   bbeq         STORAGE_KIVEC5_NEXT_DB_ACTION_BARCO_PUSH, 1, limited_sgl_case
   add		r_src_addr, STORAGE_KIVEC3ACC_DATA_ADDR, r0     // delay slot

   // VERY IMPORTANT NOTE: there are 12 TxDMA descriptors total:
   // dma_m2m_0 - dma_m2m_11, arranged in flits as follows:
   //
   // flit 9 : dma_m2m_0/dma_m2m_1/dma_m2m_2/dma_m2m_3
   // flit 10: dma_m2m_4/dma_m2m_5/dma_m2m_6/dma_m2m_7
   // flit 11: dma_m2m_8/dma_m2m_9/dma_m2m_10/dma_m2m_11
   //
   // VERY IMPORTANT NOTE: flits are not necessarily arranged intuitively,
   // that is, dma_m2m_3 in flit 9 does not immediately precede dma_m2m_4
   // in flit 10.
   //
   // Since MEM2MEM works with adjacent descriptor pair, we must ensure the
   // correctness of the pairing. For example, dma_m2m_2/dma_m2m_3 would be
   // correct, but dma_m2m_3/dma_m2m_4 would not.
   
   // Can process the entire PDMA SGL here which holds 4 addr/len pairs
   
   COMP_SGL_DMA(dma_m2m_2, dma_m2m_3, PHV_DMA_CMD_START_OFFSET(dma_m2m_4),
                d.addr0, d.len0, possible_padding_apply)
   COMP_SGL_DMA(dma_m2m_4, dma_m2m_5,PHV_DMA_CMD_START_OFFSET(dma_m2m_6),
                d.addr1, d.len1, possible_padding_apply)
   COMP_SGL_DMA(dma_m2m_6, dma_m2m_7, PHV_DMA_CMD_START_OFFSET(dma_m2m_8),
                d.addr2, d.len2, possible_padding_apply)
   COMP_SGL_DMA(dma_m2m_8, dma_m2m_9, 0,
                d.addr3, d.len3, possible_padding_apply)

src_len_remain_check:

   // Catch any driver errors here for debugging, i.e., driver did not 
   // provision the SGL correctly relative to comp output data length.
   bne          r_src_len, r0, pdma_xfer_error
   nop
      
possible_padding_apply:

   // Apply padding to the current SGL addressed by r_dst_addr if applicable.
   // The remaining length in r_sgl_len must be >= r_pad_len.
   
   add          r_pad_len, STORAGE_KIVEC3ACC_PAD_LEN, r0
   beq          r_pad_len, r0, complete_dma
   nop
   beq          r_next_dma_cmd_ptr, r0, pdma_xfer_error
   nop
   blt          r_sgl_len, r_pad_len, pdma_xfer_error
   nop

   // MEM2MEM source is pad buffer, assumed to be a 34-bit HBM address
   DMA_M2M_PTR_WRITE(mem2mem_type, CAPRI_DMA_M2M_TYPE_SRC)
   DMA_M2M_PTR_WRITE(cmdtype, CAPRI_DMA_MEM2MEM)
   DMA_M2M_PTR_WRITE(size, r_pad_len)
   DMA_M2M_PTR_WRITE(addr, STORAGE_KIVEC3ACC_PAD_BUF_ADDR)
   
   // MEM2MEM destination
   // Note: PHV flit memory is in big endian layout, so the next adjacent TxDMA
   // descriptor is at a LOWER address! Hence, the subi instruction below.
   
   subi         r_next_dma_cmd_ptr, r_next_dma_cmd_ptr, sizeof(DMA_CMD_MEM2MEM_T)
   DMA_M2M_PTR_WRITE(mem2mem_type, CAPRI_DMA_M2M_TYPE_DST)
   DMA_M2M_PTR_WRITE(cmdtype, CAPRI_DMA_MEM2MEM)
   DMA_M2M_PTR_WRITE(size, r_pad_len)
   DMA_M2M_PTR_WRITE(addr, r_dst_addr)
   DMA_M2M_PTR_WRITE(host_addr, r_dst_addr[63])
   
complete_dma:
   // Setup the start and end DMA pointers
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_11_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)

   // Exit the pipeline here
   LOAD_NO_TABLES

limited_sgl_case:
   // Only has enough dma_m2m for one, two, or three transfers
   // because an AOL pad action or SGL pad action have occupied
   // some of the DMA descriptors. So, try the best we could here.
   bbeq         STORAGE_KIVEC5_AOL_PAD_EN, 1, limited_one_sgl_case
   nop
   bbeq         STORAGE_KIVEC5_STATUS_DMA_EN, 0, limited_three_sgl_case
   nop
   COMP_SGL_DMA(dma_m2m_2, dma_m2m_3, PHV_DMA_CMD_START_OFFSET(dma_m2m_4),
                d.addr0, d.len0, possible_padding_apply)
   COMP_SGL_DMA(dma_m2m_4, dma_m2m_5, 0,
                d.addr1, d.len1, possible_padding_apply)
   b            src_len_remain_check
   nop

limited_one_sgl_case:
   // Only has enough dma_m2m for one transfer, but if status DMA
   // was not required, we can do two.
   // - status DMA took up dma_m2m_0/dma_m2m_1
   // - AOL padding in effect which took up dma_m2m_5/dma_m2m_6/dma_m2m_7/dma_m2m_8
   
   bbeq         STORAGE_KIVEC5_STATUS_DMA_EN, 0, limited_two_sgl_case
   nop
   COMP_SGL_DMA(dma_m2m_2, dma_m2m_3, 0,
                d.addr0, d.len0, possible_padding_apply)
   b            src_len_remain_check
   nop

limited_two_sgl_case:
   // Only has enough dma_m2m for two transfers
   // - status DMA not involved, freeing up dma_m2m_0/dma_m2m_1
   
   COMP_SGL_DMA(dma_m2m_0, dma_m2m_1, PHV_DMA_CMD_START_OFFSET(dma_m2m_2),
                d.addr0, d.len0, possible_padding_apply)
   COMP_SGL_DMA(dma_m2m_2, dma_m2m_3, 0,
                d.addr1, d.len1, possible_padding_apply)
   b            src_len_remain_check
   nop

limited_three_sgl_case:
   // Only has enough dma_m2m for three transfers:
   // - status DMA not involved, freeing up dma_m2m_0/dma_m2m_1
   // - SGL padding in effect which took up dma_m2m_6/dma_m2m_7/dma_m2m_8
   
   COMP_SGL_DMA(dma_m2m_0, dma_m2m_1, PHV_DMA_CMD_START_OFFSET(dma_m2m_2),
                d.addr0, d.len0, possible_padding_apply)
   COMP_SGL_DMA(dma_m2m_2, dma_m2m_3, PHV_DMA_CMD_START_OFFSET(dma_m2m_4),
                d.addr1, d.len1, possible_padding_apply)
   COMP_SGL_DMA(dma_m2m_4, dma_m2m_5, 0,
                d.addr2, d.len2, possible_padding_apply)
   b            src_len_remain_check
   nop

pdma_xfer_error:
   STORAGE_COMP_SGL_PDMA_XFER_ERROR_TRAP()
   b            complete_dma
   nop



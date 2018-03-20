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
 * Definition of a single AOL for use with tblrdp/tblwrp
 */
struct barco_aol_t {
    A0 : 64;
    O0 : 32;
    L0 : 32;
    A1 : 64;
    O1 : 32;
    L1 : 32;
    A2 : 64;
    O2 : 32;
    L2 : 32;
    next_addr : 64;
    rsvd : 64;
};


/*
 * Registers usage
 */
#define r_comp_data_len             r1  // compression output data length
#define r_total_len                 r2  // above length plus padding
#define r_pad_len                   r3  // padding length
#define r_pad_boundary              r4  // user specified padding boundary

/*
 * Registers reuse (post pad lenth computation)
 */
#define r_aol_l0_addr               r1  // pointer to AOL l0
#define r_aol_l1_addr               r2  // pointer to AOL l1

%%
   .param storage_tx_seq_comp_sgl_handler_start

storage_tx_seq_comp_status_handler_start:

   // See if should exit chain on error.
   //
   // TODO: add option to have this P4+ program copy input buffer as well as header
   // (filled with certain status and default info) to output buffer on compression error.
   seq          c1, d.err, r0
   seq.c1       c1, d.valid_bit, 1
   seq.!c1      c1, STORAGE_KIVEC5_EXIT_CHAIN_ON_ERROR, 1
   seq          c2, STORAGE_KIVEC5_NEXT_DB_EN, 1
   bcf          [c1 & c2], exit
   phvwr	p.storage_kivec5_status_err, d.err          // delay slot

   // AOL padding makes sense only when next_db_en is true, with the
   // assumption that the next db handler would act on the AOL.
   seq		c3, STORAGE_KIVEC5_DATA_LEN_FROM_DESC, 1
   cmov         r_comp_data_len, c3, STORAGE_KIVEC4_DATA_LEN, d.data_len // delay slot
   bbeq         STORAGE_KIVEC5_AOL_LEN_PAD_EN, 0, check_sgl
   phvwr	p.storage_kivec4_data_len, r_comp_data_len  // delay slot

   // AOL padding: calculate:
   // r_total_len = ((r_comp_data_len + r_pad_boundary - 1) / r_pad_boundary) * r_pad_boundary
   // r_pad_len = r_total_len - r_comp_data_len
   //
   // TODO: can AOL l0/l1/l2 specify any byte size, 
   //       or do they have to conform to some block size?
   sll          r_pad_boundary, 1, STORAGE_KIVEC5_PAD_LEN_SHIFT
   add          r_total_len, r_comp_data_len, r_pad_boundary
   sub          r_total_len, r_total_len, 1
   srl          r_total_len, r_total_len, STORAGE_KIVEC5_PAD_LEN_SHIFT
   sll          r_total_len, r_total_len, STORAGE_KIVEC5_PAD_LEN_SHIFT
   sub          r_pad_len, r_total_len, r_comp_data_len
   
   phvwr        p.data_len_len, r_comp_data_len
   phvwr        p.pad_len_len, r_pad_len

   // An assumption made here is the AOL has been set up with exactly 2 entries:
   // A0/O0/L0: input buffer, length to be modified with r_comp_data_len
   // A1/O1/L1: pad buffer, length to be modified with r_pad_len
   //
   // TODO: if L1 becomes 0, is it required to also clear A1?
   add          r_aol_l0_addr, STORAGE_KIVEC4_SGL_AOL_ADDR, offsetof(struct barco_aol_t, L0)
   add          r_aol_l1_addr, STORAGE_KIVEC4_SGL_AOL_ADDR, offsetof(struct barco_aol_t, L1)
   
   DMA_PHV2MEM_SETUP_ADDR64(data_len_len, data_len_len, r_aol_l0_addr, dma_p2m_1)
   DMA_PHV2MEM_SETUP_ADDR64(pad_len_len, pad_len_len, r_aol_l1_addr, dma_p2m_2)
   DMA_PHV2MEM_FENCE(dma_p2m_2)
   b            exit
   
check_sgl:
   
   // If compression error or SGL xfer is not applicable, end the DMA with the
   // previously doorbell/intr setup.
   seq          c4, STORAGE_KIVEC5_SGL_XFER_EN, r0  // delay slot
   bcf          [c1 | c4], complete_dma       
   nop
   
   // Set the table and program address 
   LOAD_TABLE_FOR_ADDR_PC_IMM(STORAGE_KIVEC4_SGL_AOL_ADDR, 
                              STORAGE_DEFAULT_TBL_LOAD_SIZE,
                              storage_tx_seq_comp_sgl_handler_start)

complete_dma:

   // Setup the start and end DMA pointers
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_11_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)

exit:
   LOAD_NO_TABLES

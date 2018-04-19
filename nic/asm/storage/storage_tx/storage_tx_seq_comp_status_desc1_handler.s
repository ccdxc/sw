/*****************************************************************************
 *  seq_comp_status_desc1_handler:
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s1_tbl1_k k;
struct s1_tbl1_seq_comp_status_desc1_handler_d d;
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
#define r_aol_field_p               r1  // pointer to an AOL field
#define r_aol_l1_addr               r2  // pointer to AOL L1

%%

storage_tx_seq_comp_status_desc1_handler_start:

   phvwrpair	p.{storage_kivec5_data_len...storage_kivec5_pad_len_shift}, \
   	        d.{data_len...pad_len_shift}, \
                p.{storage_kivec5_stop_chain_on_error...storage_kivec5_copy_src_dst_on_error}, \
   	        d.{stop_chain_on_error...copy_src_dst_on_error}
   phvwr	p.storage_kivec3_data_addr, d.dst_hbm_pa
   phvwr	p.pad_buf_addr_addr, d.pad_buf_pa

   bbeq		d.aol_pad_en, 0, exit
   phvwrpair	p.storage_kivec2acc_sgl_pdma_out_addr, d.sgl_pdma_out_pa, \
                p.storage_kivec2acc_sgl_vec_addr, d.sgl_vec_pa  // delay slot

   // Note that sgl_pdma_in_pa doubles as input AOL, and
   // sgl_pdma_out_pa as output AOL when aol_pad_en is set.
   //
   // An assumption made here is the input AOL has been set up with exactly 1 entry:
   // A0/O0/L0: input buffer, length to be modified with r_comp_data_len
   
   // with padding to be supplied by P4+ in
   // A1/O1/L1: pad buffer, length to be modified with r_pad_len
   add          r_aol_field_p, d.sgl_pdma_in_pa, \
                SIZE_IN_BYTES(offsetof(struct barco_aol_le_t, L0))
   DMA_PHV2MEM_SETUP_ADDR64(data_len_len, data_len_len, r_aol_field_p, dma_p2m_2)
   
   add          r_aol_field_p, d.sgl_pdma_in_pa, \
                SIZE_IN_BYTES(offsetof(struct barco_aol_le_t, A1))
   DMA_PHV2MEM_SETUP_ADDR64(pad_buf_addr_addr, pad_buf_addr_addr, r_aol_field_p, dma_p2m_3)
   
   add          r_aol_field_p, d.sgl_pdma_in_pa, \
                SIZE_IN_BYTES(offsetof(struct barco_aol_le_t, L1))
   DMA_PHV2MEM_SETUP_ADDR64(pad_len_len, pad_len_len, r_aol_field_p, dma_p2m_4)

   // And output AOL has exactly one entry to be modified
   add          r_aol_field_p, d.sgl_pdma_out_pa, \
                SIZE_IN_BYTES(offsetof(struct barco_aol_le_t, L0))
   DMA_PHV2MEM_SETUP_ADDR64(total_len_len, total_len_len, r_aol_field_p, dma_p2m_5)
   DMA_PHV2MEM_FENCE(dma_p2m_5)
   
exit:
   CLEAR_TABLE_VALID_e(1)


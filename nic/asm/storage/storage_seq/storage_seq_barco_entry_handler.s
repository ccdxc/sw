/*****************************************************************************
 *  seq_barco_entry_handler: Handle the Barco XTS entry in sequencer. Form the
 *                           DMA command to copy the Barco XTS descriptor as
 *                           part of the push operation in the next stage.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s1_tbl_k k;
struct s1_tbl_seq_barco_entry_handler_d d;
struct phv_ p;

/*
 * Registers usage
 */
#define r_num_descs                     r1  // number of Barco descriptors
#define r_rl_len                        r3  // rate limit length for SEQ_RATE_LIMIT_...()

%%
   .param storage_seq_barco_ring_pndx_pre_read0

storage_seq_barco_entry_handler:

   // Update the K+I vector with the barco descriptor size to be used
   // when calculating the offset for the push operation.
   // phvwrpair limits destination p[] to 64 bits per.
   phvwrpair	p.seq_kivec4_barco_ring_addr, d.barco_ring_addr[33:0], \
                p.seq_kivec4_barco_pndx_shadow_addr, d.barco_pndx_shadow_addr[33:0]
   phvwrpair	p.seq_kivec4_barco_desc_size, d.barco_desc_size[3:0], \
                p.seq_kivec4_barco_pndx_size, d.barco_pndx_size[2:0]
   phvwr	p.seq_kivec4_barco_desc_addr, d.barco_desc_addr
                
   seq          c1, d.batch_mode[0], 1
   cmov         r_num_descs, c1, d.batch_size, 1
   phvwrpair	p.seq_kivec4_barco_ring_size, d.barco_ring_size[4:0], \
                p.seq_kivec4_barco_num_descs, r_num_descs
   
   // Set rate limiting if applicable
if0:   
   bbeq         d.rate_limit_en, 0, endif0
   SEQ_RATE_LIMIT_ENABLE_CHECK(d.rate_limit_src_en, c3) // delay slot
   SEQ_RATE_LIMIT_DATA_LEN_LOAD_c(c3, d.src_data_len)
   SEQ_RATE_LIMIT_ENABLE_CHECK(d.rate_limit_dst_en, c4)
   SEQ_RATE_LIMIT_DATA_LEN_ADD_c(c4, d.dst_data_len)
   SEQ_RATE_LIMIT_SET_c(c0)
endif0:
    
   DMA_PHV2MEM_SETUP_ADDR34(barco_doorbell_data_p_ndx, barco_doorbell_data_p_ndx,
                            d.barco_pndx_addr[33:0], dma_p2m_19)
   DMA_PHV2MEM_FENCE(dma_p2m_19)
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad,
                 dma_p2m_19_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)
   SEQ_METRICS_SET(next_db_rung)
   SEQ_METRICS_VAL_SET(seq_hw_bytes, d.src_data_len)

   // Advance to a common stage for executing table lock read to get the
   // Barco ring pindex.
   LOAD_TABLE_NO_LKUP_PC_IMM_e(0, storage_seq_barco_ring_pndx_pre_read0)


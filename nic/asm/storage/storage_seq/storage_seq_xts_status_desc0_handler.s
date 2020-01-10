/*****************************************************************************
 *  seq_xts_status_desc0_handler: Handle the XTS status descriptor entry in the
 *                                sequencer.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s1_tbl_k k;
struct s1_tbl_seq_xts_status_desc0_handler_d d;
struct phv_ p;

/*
 * Registers usage
 * CAUTION: r1 is also implicitly used by LOAD_TABLEx_FOR_ADDR_PC_IMM()
 *          r7 is also implicitly used by SEQUENCER_DOORBELL_RING() and
 *             PCI_SET_INTERRUPT_ADDR_DMA()
 */
#define r_status_src                r3  // source status address
#define r_status_len                r4  // status transfer length

%%
   .param storage_seq_xts_status_handler

storage_tx_seq_xts_status_desc0_handler:

   // Order of evaluation of next doorbell and interrupts
   // 1. If next_db_en is set => ring the next doorbell and don't raise interrupt
   // 2. If next_db_en is not set and intr_en is set => raise interrupt

   // Note: if a compression error arises and status_dma_en is set,
   // interrupts (if set) will also be raised
   PCI_SET_INTERRUPT_DATA()
   phvwr        p.{seq_kivec5xts_status_dma_en...seq_kivec5xts_rate_limit_en}, \
   	        d.{status_dma_en...rate_limit_en}
   bbeq         d.next_db_en, 0, intr_check
   phvwr	p.seq_kivec10_intr_addr, d.intr_addr    // delay slot
   
   SEQ_METRICS_SET(next_db_rung)

   // If doorbell is actually a Barco push action, handle accordingly.
   // Note that d.next_db_data in this case is really d.barco_desc_addr
   bbeq		d.next_db_action_barco_push, 0, next_db_ring
   phvwr	p.seq_kivec7xts_comp_desc_addr, d.next_db_data // delay slot
                
   DMA_PHV2MEM_SETUP_ADDR34(barco_doorbell_data_p_ndx, barco_doorbell_data_p_ndx,
                            d.barco_pndx_addr, dma_p2m_19)
   DMA_PHV2MEM_FENCE(dma_p2m_19)
   
   // Note that d.next_db_addr in this case is really d.barco_ring_addr
   // phvwrpair limits destination p[] to 64 bits per.
   phvwrpair	p.seq_kivec4_barco_ring_addr, d.next_db_addr[33:0], \
                p.{seq_kivec4_barco_pndx_shadow_addr...seq_kivec4_barco_ring_size}, \
                d.{barco_pndx_shadow_addr...barco_ring_size}
   phvwrpair    p.seq_kivec4_barco_desc_addr, d.next_db_data, \
                p.seq_kivec4_barco_num_descs, d.barco_num_descs[9:0]
                
possible_status_dma:

   // Set up further status xfer if applicable
   bbeq         d.status_dma_en, 0, possible_tbl_load
   add          r_status_len, r0, d.status_len    // delay slot
   
   // Set up the status DMA:
   add          r_status_src, d.status_addr0, d.status_offset0
   DMA_MEM2MEM_NO_LIF_SETUP(CAPRI_DMA_M2M_TYPE_SRC, r_status_src,
                            r_status_len, dma_m2m_0)
   DMA_MEM2MEM_NO_LIF_SETUP(CAPRI_DMA_M2M_TYPE_DST, d.status_addr1, 
                            r_status_len, dma_m2m_1)
   SEQ_METRICS_SET(status_pdma_xfers)

possible_tbl_load:

   // Status addr is not to be read when crypto chaining is only
   // used for raising interrupt + doorbell (since the crypto
   // engine is not capable of generating 2 "opaque" writes on its own)
if2:   
   sne          c1, d.status_addr0, r0
   bcf          [c1], endif2
   CLEAR_TABLE0                                         // delay slot

   // Setup the start and end DMA pointers
   DMA_PTR_SETUP_e(dma_p2m_0_dma_cmd_pad,
                   dma_p2m_19_dma_cmd_eop,
                   p4_txdma_intr_dma_cmd_ptr)
endif2:

   // Set the table and program address 
   LOAD_TABLE_FOR_ADDR_PC_IMM(d.status_addr0, STORAGE_TBL_LOAD_SIZE_64_BITS,
                              storage_seq_xts_status_handler)

intr_check:
   // Check if interrupt is enabled
   bbeq		d.intr_en, 0, possible_status_dma
   nop

intr_raise:
   // Raise interrupt based on addr/data provided in descriptor
   PCI_SET_INTERRUPT_ADDR_DMA(d.intr_addr, dma_p2m_19)
   b            possible_status_dma
   SEQ_METRICS_SET(interrupts_raised)                   // delay slot

next_db_ring:
                            
   // Upon arrival at this label, d.next_db_en is true and d.next_db_action_barco_push
   // is false, which means the TxDMA descriptors normally reserved for Barco push are
   // free for use. Taking advantage of this, we should be able to generate both
   // doorbell as well as interrupt.
if0:   
   bbeq		d.intr_en, 0, endif0
   nop
   SEQUENCER_DOORBELL_RING_NO_FENCE(dma_p2m_17)
   b            intr_raise
   nop
endif0:   
   SEQUENCER_DOORBELL_RING(dma_p2m_19)
   b            possible_status_dma
   nop


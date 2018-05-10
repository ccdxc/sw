/*****************************************************************************
 *  seq_comp_status_desc_handler: Handle the compression status descriptor entry in the
 *                         sequencer. This involves:
 *                          1. processing status to see if operation succeeded
 *                          2. breaking up the compressed data into the
 *                             destination SGL provided in the descriptor
 *                         In this stage, load the status entry for next stage
 *                         and save the other fields into I vector.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s1_tbl_k k;
struct s1_tbl_seq_comp_status_desc0_handler_d d;
struct phv_ p;

%%
   .param storage_seq_comp_status_handler

storage_seq_comp_status_desc0_handler_start:

   // Order of evaluation of next doorbell and interrupts
   // 1. If next_db_en is set => ring the next doorbell and don't raise interrupt
   // 2. If next_db_en is not set and intr_en is set => raise interrupt
   //
   // Note: if a compression error arises and status_dma_en is set,
   // interrupts (if set) will also be raised
   PCI_SET_INTERRUPT_DATA()

   // Store the various parts of the descriptor in the K+I vectors for later use
   // Check if next doorbell is to be enabled
   bbeq		d.next_db_en, 0, intr_check
   phvwrpair	p.seq_kivec5_intr_addr, d.intr_addr, \
                p.{seq_kivec5_status_dma_en...seq_kivec5_next_db_action_barco_push}, \
   	        d.{status_dma_en...next_db_action_barco_push}   // delay slot

   // if doorbell is actually a Barco push action, handle accordingly
   bbeq		d.next_db_action_barco_push, 0, next_db_ring
   sll          r7, 1, d.barco_desc_size        // delay slot
                
   // Setup the source of the mem2mem DMA into DMA cmd 1.
   // Note: next_db_data doubles as barco_desc_addr in this case
   DMA_MEM2MEM_NO_LIF_SETUP(CAPRI_DMA_M2M_TYPE_SRC, d.next_db_data, r7,
                            dma_m2m_19)

   // Setup the destination of the mem2mem DMA into DMA cmd 2 (just fill
   // the size).
   DMA_MEM2MEM_NO_LIF_SETUP_REG_ADDR(CAPRI_DMA_M2M_TYPE_DST, r0, r7,
                                     dma_m2m_20)

   // Copy the data for the doorbell into the PHV and setup a DMA command
   // to ring it. Form the doorbell DMA command in this stage as opposed 
   // the push stage (as is the norm) to avoid carrying the doorbell address 
   // in K+I vector.
   DMA_PHV2MEM_SETUP_ADDR34(barco_doorbell_data_p_ndx, barco_doorbell_data_p_ndx,
                            d.barco_pndx_addr, dma_p2m_21)
   DMA_PHV2MEM_FENCE(dma_p2m_21)
   
   // Note that d.next_db_addr in this case is really d.barco_ring_addr.
   // phvwrpair limits destination p[] to 64 bits per.
   phvwrpair	p.seq_kivec4_barco_ring_addr, d.next_db_addr[33:0], \
                p.{seq_kivec4_barco_pndx_shadow_addr...seq_kivec4_barco_ring_size}, \
                d.{barco_pndx_shadow_addr...barco_ring_size}
   b            status_dma_setup
   
   // phvwrpair limits destination p[] to 64 bits per.
   phvwri       p.seq_kivec4_barco_num_descs, 1     // delay slot

next_db_ring:
                            
   // Ring the sequencer doorbell based on addr/data provided in the descriptor
   SEQUENCER_DOORBELL_RING(dma_p2m_21)

status_dma_setup:

   // Set up status xfer if applicable
   bbeq         d.status_dma_en, 0, tbl_load
   add          r3, r0, d.status_len    // delay slot
   
   // Set up the status DMA:
   DMA_MEM2MEM_NO_LIF_SETUP(CAPRI_DMA_M2M_TYPE_SRC, d.status_addr0, 
                            r3, dma_m2m_0)
   DMA_MEM2MEM_NO_LIF_SETUP(CAPRI_DMA_M2M_TYPE_DST, d.status_addr1, 
                            r3, dma_m2m_1)
   
tbl_load:
   // Setup the start and end DMA pointers
   // Set the table and program address 
   LOAD_TABLE_FOR_ADDR_PC_IMM(d.status_addr0, STORAGE_DEFAULT_TBL_LOAD_SIZE,
                              storage_seq_comp_status_handler)

intr_check:
   // Check if interrupt is enabled
   bbeq		d.intr_en, 0, status_dma_setup
   nop

   // Raise interrupt based on addr/data provided in descriptor
   PCI_SET_INTERRUPT_ADDR_DMA(d.intr_addr, dma_p2m_21)
   b            status_dma_setup
   nop



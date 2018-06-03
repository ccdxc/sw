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
 * Registers usage
 * CAUTION: r1 is also implicitly used by LOAD_TABLE1_FOR_ADDR_PC_IMM()
 */
#define r_comp_data_len             r1  // compression output data length
#define r_total_len                 r2  // above length plus padding
#define r_pad_len                   r3  // padding length
#define r_pad_boundary              r4  // user specified padding boundary
#define r_num_blks                  r5  // number of hash blocks
#define r_last_sgl_p                r6  // pointer to last SGL
#define r_sgl_field_p               r7  // pointer to an SGL field

/*
 * Registers reuse, post padding calculations
 */
#define r_last_blk_len              r1  // length of last block
#define r_status                    r7  // comp status, briefly used at beginning

%%
    .param storage_seq_barco_ring_pndx_read
    .param storage_seq_comp_aol_pad_prep
    .param storage_seq_comp_sgl_pdma_xfer
    .param storage_seq_comp_sgl_pad_only_xfer

storage_seq_comp_status_handler:
    
     CLEAR_TABLE0
     
    // c6 indicates whether padding is enabled, for use by possible_sgl_pdma_xfer
    setcf       c6, [!c0]
    
    // bit 15: valid bit, bits 14-12: error bits
    add         r_status, d.status, r0
    smeqh       c4, r_status, 0xf000, 0x8000
    bcf         [!c4], comp_error
   
    // Note: output_data_len contains compressed data length plus header length.
    
    seq	        c3, SEQ_KIVEC5_DATA_LEN_FROM_DESC, 1        // delay slot
    cmov        r_comp_data_len, c3, SEQ_KIVEC5_DATA_LEN, d.output_data_len
    phvwr	p.seq_kivec5_data_len, r_comp_data_len
   
    // Preliminary padding calculations:
    // r_num_blks = (r_comp_data_len + r_pad_boundary - 1) / r_pad_boundary)
    // r_total_len = r_num_blks * r_pad_boundary
    // r_pad_len = r_total_len - r_comp_data_len
   
    sll         r_pad_boundary, 1, SEQ_KIVEC4_PAD_BOUNDARY_SHIFT
    add         r_num_blks, r_comp_data_len, r_pad_boundary
    sub         r_num_blks, r_num_blks, 1
    srl         r_num_blks, r_num_blks, SEQ_KIVEC4_PAD_BOUNDARY_SHIFT
    sll         r_total_len, r_num_blks, SEQ_KIVEC4_PAD_BOUNDARY_SHIFT
    sub         r_pad_len, r_total_len, r_comp_data_len
    sub         r_last_blk_len, r_pad_boundary, r_pad_len

    phvwrpair   p.comp_last_blk_len_len, r_last_blk_len.wx, \
                p.comp_pad_len_len, r_pad_len.wx
    
    // Note that both SGL padding and AOL padding may be enabled, for the
    // following use case: compress-pad-encrypt where the compressed output
    // data (padded) is saved while at the same time passed to Barco
    // for encryption (compression uses SGL format while encryption uses AOL).
                     
    bbeq        SEQ_KIVEC5_AOL_PAD_EN, 0, possible_sgl_padding
    phvwrpair   p.seq_kivec8_pad_len, r_pad_len, \
                p.seq_kivec8_last_blk_len, r_last_blk_len       // delay slot
    
aol_padding:
    
    // AOL padding enabled: handle in the next stage due to low availability
    // of k-vec space which forces usage of a stage_2_stage (seq_kivec6).
    LOAD_TABLE_NO_LKUP_PC_IMM(3, storage_seq_comp_aol_pad_prep) 

    // Tell possible_sgl_pdma_xfer that padding is enabled
    setcf       c6, [c0]

possible_sgl_padding:
    bbeq        SEQ_KIVEC5_SGL_PAD_EN, 0, possible_per_block_descs
    phvwr       p.seq_kivec3_num_blks, r_num_blks        // delay slot

    // SGL padding enabled:
    // Given a vector of SGLs, each prefilled with exactly one block addr and len,
    // i.e., addr0/len0 specifies one block of data, find the last applicable SGL
    // and apply padding.
   
    // Tell possible_sgl_pdma_xfer that padding is enabled
    setcf       c6, [c0]
    
if0:
    beq         r_pad_len, r0, endif0
    sub         r_last_sgl_p, r_num_blks, 1     // delay slot
    add         r_last_sgl_p, SEQ_KIVEC2_SGL_VEC_ADDR, \
                r_last_sgl_p, BARCO_SGL_DESC_SIZE_SHIFT

    // Set up DMA for the following:
    // last_sgl_p->len0 = r_last_blk_len
    // last_sgl_p->addr1 = comp_pad_buf
    // last_sgl_p->len1 = r_pad_len
    
    add         r_sgl_field_p, r_last_sgl_p, \
                SIZE_IN_BYTES(offsetof(struct barco_sgl_le_t, len0))
    DMA_PHV2MEM_SETUP_ADDR64(comp_last_blk_len_len, comp_last_blk_len_len,
                             r_sgl_field_p, dma_p2m_2)
    
    add         r_sgl_field_p, r_last_sgl_p, \
                SIZE_IN_BYTES(offsetof(struct barco_sgl_le_t, addr1))
    DMA_PHV2MEM_SETUP_ADDR64(pad_buf_addr_addr, pad_buf_addr_addr,
                             r_sgl_field_p, dma_p2m_3)
    
    add         r_sgl_field_p, r_last_sgl_p, \
                SIZE_IN_BYTES(offsetof(struct barco_sgl_le_t, len1))
    DMA_PHV2MEM_SETUP_ADDR64(comp_pad_len_len, comp_pad_len_len,
                             r_sgl_field_p, dma_p2m_4)

endif0:

possible_per_block_descs:

    // In the per-block hash or encryption case, we now indicate to
    // storage_seq_barco_chain_action the correct number of descriptors.
    
    bbeq        SEQ_KIVEC5_DESC_VEC_PUSH_EN, 0, possible_sgl_pdma_xfer
    nop
    phvwr       p.seq_kivec4_barco_num_descs, r_num_blks
    
possible_sgl_pdma_xfer:

    // PDMA compressed data only for non-error case (c4 was set)
    seq.c4      c4, SEQ_KIVEC5_SGL_PDMA_EN, 1
    bcf         [!c4], possible_barco_push
    phvwr.c6    p.seq_kivec3_pad_len, r_pad_len          // delay slot
    bbeq        SEQ_KIVEC5_SGL_PDMA_PAD_ONLY, 0, sgl_pdma_xfer_full
    nop
    
    // sgl_pad_en must have been enabled, the result of which
    // is referenced here for PDMA transfer of the pad data.
    
    bbeq        SEQ_KIVEC5_SGL_PAD_EN, 0, pdma_pad_only_error
    nop
    LOAD_TABLE2_FOR_ADDR_PC_IMM(r_last_sgl_p, 
                                STORAGE_DEFAULT_TBL_LOAD_SIZE,
                                storage_seq_comp_sgl_pad_only_xfer)
    b           possible_barco_push
    nop
    
sgl_pdma_xfer_full:
                                
    // PDMA compressed data to user buffers specified in SGL
    LOAD_TABLE1_FOR_ADDR_PC_IMM(SEQ_KIVEC2_SGL_PDMA_DST_ADDR, 
                                STORAGE_DEFAULT_TBL_LOAD_SIZE,
                                storage_seq_comp_sgl_pdma_xfer)
possible_barco_push:

    // If Barco ring push is applicable, execute table lock read
    // to get the current ring pindex. Note that this must be done
    // in the same stage as storage_seq_barco_entry_handler()
    // which is stage 3.
    bbeq        SEQ_KIVEC5_NEXT_DB_ACTION_BARCO_PUSH, 0, all_dma_complete
    nop

    LOAD_TABLE_NO_LKUP_PC_IMM(0, storage_seq_barco_ring_pndx_read)
    
all_dma_complete:

    // Setup the start and end DMA pointers
    DMA_PTR_SETUP_e(dma_p2m_0_dma_cmd_pad,
                    dma_p2m_21_dma_cmd_eop,
                    p4_txdma_intr_dma_cmd_ptr)

comp_error:

    // If next_db_en and !stop_chain_on_error then ring_db
    seq         c5, SEQ_KIVEC5_NEXT_DB_EN, 1
    bbeq.c5     SEQ_KIVEC5_STOP_CHAIN_ON_ERROR, 0, possible_sgl_pdma_xfer
    nop

    // cancel any barco push prep
    DMA_CMD_CANCEL(dma_p2m_21)
   
    // else if intr_en then complete any status DMA and 
    // override doorbell to raising an interrupt
    bbeq        SEQ_KIVEC5_INTR_EN, 0, all_dma_complete
    nop

    PCI_SET_INTERRUPT_ADDR_DMA(SEQ_KIVEC5_INTR_ADDR,
                               dma_p2m_21)
    b           all_dma_complete
    nop

pdma_pad_only_error:

   // PDMA pad-only requires sgl_pad_en to also be enabled
   // (the sgl_pad_en operation provides the necessary SGL
   // for the calculation and resulting PDMA transfer of the pad data)
   
   SEQ_COMP_SGL_PDMA_PAD_ONLY_ERROR_TRAP()
   b            possible_barco_push
   nop


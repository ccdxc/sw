/*****************************************************************************
 *  seq_comp_sgl_pad_only_xfer: on entry, d points to the last Barco SGL descriptor
 *                         where the comp engine has written the last chunk of
 *                         output compressed data (to d.addr0). Here, we'll 
 *                         calculate how much it has written and transfer the
 *                         remaining buffer with pad data.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s3_tbl2_k k;
struct s3_tbl2_seq_comp_sgl_pad_only_xfer_d d;
struct phv_ p;

/*
 * Registers usage
 * Note: avoid r1 as it is implicitly used by DMA_MEM2MEM_SETUP() 
 */
#define r_pad_len                   r3  // padding length
#define r_pad_dest                  r4  // pointer to destination to apply padding
#define r_comp_buf_addr             r5  // pointer to comp buffer address
#define r_src_qaddr                 r7  // for SEQ_METRICS_TABLE_COMMIT

%%
    SEQ_METRICS_PARAMS()

storage_seq_comp_sgl_pad_only_xfer:

    // Note: this function is a common entry point for both SGL PDMA xfer
    // and CP header update.
    
    bbeq        SEQ_KIVEC5_CP_HDR_UPDATE_EN, 0, possible_sgl_pad
    seq         c3, SEQ_KIVEC5_INTEG_DATA_NULL_EN, 1            // delay slot
    phvwr.c3	p.comp_hdr_cksum, r0
    
    // No fence needed for CP header update since any prior PDMA mem2mem
    // would only be pad data xfer and, presumably, be outside of
    // the CP header area.
    add         r_comp_buf_addr, SEQ_KIVEC3_COMP_BUF_ADDR, r0
    DMA_PHV2MEM_SETUP(comp_hdr_cksum,
                      comp_hdr_version,
                      r_comp_buf_addr, dma_p2m_18)
    SEQ_METRICS_SET(cp_header_updates)

possible_sgl_pad:    
    bbeq        SEQ_KIVEC5_SGL_PAD_EN, 0, exit
    
    // dst_addr to apply padding = d.addr0 + last_blk_len
    
    add         r_pad_len, SEQ_KIVEC3_PAD_LEN, r0               // delay slot
    beq         r_pad_len, r0, exit
    add         r_pad_dest, d.addr0, SEQ_KIVEC3_LAST_BLK_LEN    // delay slot

    seq         c1, SEQ_KIVEC3_SGL_TUPLE_NO, 1
    add.c1      r_pad_dest, d.addr1, SEQ_KIVEC3_LAST_BLK_LEN
    seq         c1, SEQ_KIVEC3_SGL_TUPLE_NO, 2
    add.c1      r_pad_dest, d.addr2, SEQ_KIVEC3_LAST_BLK_LEN
    
    DMA_MEM2MEM_NO_LIF_SETUP(CAPRI_DMA_M2M_TYPE_SRC, SEQ_KIVEC5_PAD_BUF_ADDR,
                             r_pad_len, dma_m2m_4)
    DMA_MEM2MEM_NO_LIF_SETUP_REG_ADDR(CAPRI_DMA_M2M_TYPE_DST, r_pad_dest,
                                      r_pad_len, dma_m2m_5)
    SEQ_METRICS_SET(sgl_pad_only_xfers)                 // delay slot

exit:

    // Relaunch stats commit for table 2
    SEQ_METRICS1_TABLE2_COMMIT_e(SEQ_KIVEC5_SRC_QADDR)

